#include "ehupch.h"
#include "DashboardLayer.h"
#include "DashboardStats.h"
#include "ImGuiWindowVisibility.h"
#include "Platform/IO/Input.h"
#include "Core/KeyCodes.h"
#include "imgui.h"
#include <cfloat>

namespace Ehu {

	// 根据帧时间着色：绿 / 黄 / 红
	static void FrameTimeColor(float frameMs, float& r, float& g, float& b) {
		if (frameMs <= 0.0f) { r = g = b = 0.5f; return; }
		if (frameMs < 16.67f) { r = 0.2f; g = 0.8f; b = 0.2f; return; }  // < 60fps 绿
		if (frameMs < 33.33f) { r = 0.9f; g = 0.8f; b = 0.2f; return; }  // 30~60 黄
		r = 0.95f; g = 0.3f; b = 0.2f;  // > 30fps 红
	}

	void DashboardLayer::OnImGuiRender() {
		using namespace ImGuiWindowVisibility;
		// F3 切换 Dashboard 显示（与菜单栏一致）
		{
			static bool s_WasF3 = false;
			bool f3 = Input::IsKeyPressed(Key::F3);
			if (f3 && !s_WasF3)
				ShowDashboard = !ShowDashboard;
			s_WasF3 = f3;
		}

		if (!ShowDashboard)
			return;

		// 再次打开时回到默认停靠位置（从关闭变为打开时强制位置）
		{
			static bool s_WasVisible = false;
			bool justOpened = ShowDashboard && !s_WasVisible;
			if (justOpened) {
				float x, y;
				GetDefaultWindowPos("Dashboard", x, y);
				ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
			}
			s_WasVisible = ShowDashboard;
		}

		DashboardStats& stats = DashboardStats::Get();

		if (!ImGui::Begin("Dashboard", &ShowDashboard, ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}

		const int historyCount = static_cast<int>(stats.HistoryCount < DashboardStats::kHistoryFrames ? stats.HistoryCount : DashboardStats::kHistoryFrames);
		const float frameMs = stats.FrameTimeMs;
		const float fps = frameMs > 0.0f ? 1000.0f / frameMs : 0.0f;

		// ---------- Timing ----------
		if (ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen)) {
			// 首行：Frame Time + FPS，带颜色
			float cr, cg, cb;
			FrameTimeColor(frameMs, cr, cg, cb);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(cr, cg, cb, 1.0f));
			ImGui::Text("Frame Time: %.3f ms  |  FPS: %.1f", frameMs, fps);
			ImGui::PopStyleColor();

			// CPU / GPU 瓶颈：用进度条表示本帧占比（CPU + GPU 归一化）
			float totalMs = stats.CpuTimeMs + (stats.GpuTimeMs >= 0.0f ? stats.GpuTimeMs : 0.0f);
			if (totalMs > 0.001f) {
				float cpuFrac = stats.CpuTimeMs / totalMs;
				float gpuFrac = (stats.GpuTimeMs >= 0.0f ? stats.GpuTimeMs : 0.0f) / totalMs;
				ImGui::TextUnformatted("Bottleneck (this frame):");
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
				ImGui::ProgressBar(cpuFrac, ImVec2(-1, 0), "CPU");
				ImGui::PopStyleColor();
				if (stats.GpuTimeMs >= 0.0f) {
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.4f, 1.0f));
					ImGui::ProgressBar(gpuFrac, ImVec2(-1, 0), "GPU");
					ImGui::PopStyleColor();
				}
			}
			if (stats.GpuTimeMs >= 0.0f)
				ImGui::Text("CPU: %.3f ms  |  GPU: %.3f ms", stats.CpuTimeMs, stats.GpuTimeMs);
			else
				ImGui::Text("CPU: %.3f ms  |  GPU: N/A", stats.CpuTimeMs);

			// 子系统耗时：表格 + 小进度条（相对本帧）
			if (ImGui::BeginTable("Subsystems", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Subsystem", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthFixed, 60.0f);
				ImGui::TableHeadersRow();
				auto row = [&](const char* name, float ms) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(name);
					float frac = (frameMs > 0.0f) ? (ms / frameMs) : 0.0f;
					if (frac > 0.0f)
						ImGui::ProgressBar(frac, ImVec2(80, 0), "");
					ImGui::SameLine(0, 4);
					ImGui::TableNextColumn();
					ImGui::Text("%.3f", ms);
				};
				row("Update", stats.UpdateMs);
				row("Physics", stats.PhysicsMs);
				row("Render Submit", stats.RenderSubmitMs);
				row("Scripting", stats.ScriptingMs);
				ImGui::EndTable();
			}

			// 折线图：ImGui 的 PlotLines 会生成折线顶点到 DrawList
			if (historyCount > 0) {
				ImGui::Spacing();
				ImGui::TextUnformatted("Frame Time (ms)");
				ImGui::PlotLines("##FrameTime", stats.FrameTimeHistory, historyCount, 0, nullptr, 0.0f, 50.0f, ImVec2(-1, 70));
				if (stats.GpuTimeMs >= 0.0f) {
					ImGui::TextUnformatted("GPU Time (ms)");
					ImGui::PlotLines("##GpuTime", stats.GpuTimeHistory, historyCount, 0, nullptr, 0.0f, 50.0f, ImVec2(-1, 70));
				}
			}
		}

		// ---------- Rendering ----------
		if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("RenderingStats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 80.0f);
				ImGui::TableHeadersRow();
#define ROW(label, fmt, ...) do { ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label); ImGui::TableNextColumn(); ImGui::Text(fmt, __VA_ARGS__); } while(0)
				ROW("Draw Calls 2D", "%u", stats.DrawCalls2D);
				ROW("Draw Calls 3D", "%u", stats.DrawCalls3D);
				ROW("Draw Calls Total", "%u", stats.DrawCalls2D + stats.DrawCalls3D);
				ROW("Triangles 2D", "%u", stats.Triangles2D);
				ROW("Triangles 3D", "%u", stats.Triangles3D);
				ROW("Triangles Total", "%u", stats.Triangles2D + stats.Triangles3D);
				uint32_t vertApprox = stats.Vertices2D + stats.Vertices3D;
				ROW("Vertices", "%u", vertApprox > 0 ? vertApprox : (stats.Triangles2D + stats.Triangles3D) * 3);
				ROW("Texture Bindings", "%u", stats.TextureBindings);
				ROW("Shader Switches", "%u", stats.ShaderSwitches);
#undef ROW
				ImGui::EndTable();
			}
		}

		// ---------- Memory & Resources ----------
		if (ImGui::CollapsingHeader("Memory & Resources", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("MemoryStats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Resource", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableHeadersRow();
				ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("VRAM");
				ImGui::TableNextColumn();
				if (stats.VramBytes > 0) ImGui::Text("%.2f MB", stats.VramBytes / (1024.0 * 1024.0)); else ImGui::TextUnformatted("N/A");
				ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Heap");
				ImGui::TableNextColumn();
				if (stats.HeapBytes > 0) ImGui::Text("%.2f MB", stats.HeapBytes / (1024.0 * 1024.0)); else ImGui::TextUnformatted("N/A");
				ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted("Active Entities");
				ImGui::TableNextColumn(); ImGui::Text("%u", stats.ActiveEntities);
				ImGui::EndTable();
			}
			if (stats.HeapBytes > 0 && historyCount > 0) {
				ImGui::Spacing();
				ImGui::TextUnformatted("Heap (MB)");
				ImGui::PlotLines("##HeapMb", stats.HeapMbHistory, historyCount, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(-1, 70));
			}
		}

		ImGui::End();
	}

} // namespace Ehu
