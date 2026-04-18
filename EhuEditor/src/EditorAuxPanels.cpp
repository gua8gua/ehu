#include "EditorAuxPanels.h"
#include "Core/Application.h"
#include "Core/RuntimeStats.h"
#include "ImGui/DashboardStats.h"
#include "Renderer/RenderQueue.h"
#include <imgui.h>
#include <cfloat>
#include <cstring>

namespace Ehu {

	void GetDefaultEditorPanelPos(const char* windowName, float& outX, float& outY) {
		ImGuiViewport* vp = ImGui::GetMainViewport();
		const float menuBarHeight = 21.0f;
		const float pad = 20.0f;
		const float startY = vp->WorkPos.y + menuBarHeight + pad;

		if (std::strcmp(windowName, "Stats") == 0) {
			outX = vp->WorkPos.x + pad;
			outY = startY;
		} else if (std::strcmp(windowName, "Dashboard") == 0) {
			outX = vp->WorkPos.x + pad + 280.0f;
			outY = startY;
		} else if (std::strcmp(windowName, "Console") == 0) {
			outX = vp->WorkPos.x + pad;
			outY = vp->WorkPos.y + vp->WorkSize.y - 260.0f;
		} else {
			outX = vp->WorkPos.x + pad;
			outY = startY;
		}
	}

	static void FrameTimeColor(float frameMs, float& r, float& g, float& b) {
		if (frameMs <= 0.0f) {
			r = g = b = 0.5f;
			return;
		}
		if (frameMs < 16.67f) {
			r = 0.2f;
			g = 0.8f;
			b = 0.2f;
			return;
		}
		if (frameMs < 33.33f) {
			r = 0.9f;
			g = 0.8f;
			b = 0.2f;
			return;
		}
		r = 0.95f;
		g = 0.3f;
		b = 0.2f;
	}

	void RenderEditorStatsPanel(bool& showOpen) {
		if (!showOpen)
			return;

		{
			static bool s_WasVisible = false;
			const bool justOpened = showOpen && !s_WasVisible;
			if (justOpened) {
				float x, y;
				GetDefaultEditorPanelPos("Stats", x, y);
				ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
			}
			s_WasVisible = showOpen;
		}

		if (!ImGui::Begin("Stats", &showOpen, ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		Application& app = Application::Get();
		const float fps = app.GetFPS();
		const float dt = app.GetDeltaTime();
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Delta: %.3f ms", dt * 1000.0f);

		ImGui::Separator();
		if (app.IsMainWindowSceneRenderingEnabled()) {
			const RenderQueue* queue = app.GetRenderQueue();
			if (queue) {
				const RenderStats& s = queue->GetLastFrameStats();
				ImGui::Text("Draw Calls 2D: %u", s.DrawCalls2D);
				ImGui::Text("Draw Calls 3D: %u", s.DrawCalls3D);
				ImGui::Text("Triangles 2D: %u", s.Triangles2D);
				ImGui::Text("Triangles 3D: %u", s.Triangles3D);
			}
		} else {
			const RuntimeStats& stats = RuntimeStats::Get();
			ImGui::TextUnformatted("Source: Scene Viewport");
			ImGui::Text("Draw Calls 2D: %u", stats.DrawCalls2D);
			ImGui::Text("Draw Calls 3D: %u", stats.DrawCalls3D);
			ImGui::Text("Triangles 2D: %u", stats.Triangles2D);
			ImGui::Text("Triangles 3D: %u", stats.Triangles3D);
		}
		ImGui::End();
	}

	void RenderEditorDashboardPanel(bool& showOpen) {
		if (!showOpen)
			return;

		{
			static bool s_WasVisible = false;
			const bool justOpened = showOpen && !s_WasVisible;
			if (justOpened) {
				float x, y;
				GetDefaultEditorPanelPos("Dashboard", x, y);
				ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
			}
			s_WasVisible = showOpen;
		}

		DashboardStats& stats = DashboardStats::Get();

		if (!ImGui::Begin("Dashboard", &showOpen, ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}

		const int historyCount = static_cast<int>(stats.HistoryCount < DashboardStats::kHistoryFrames ? stats.HistoryCount : DashboardStats::kHistoryFrames);
		const float frameMs = stats.FrameTimeMs;
		const float fps = frameMs > 0.0f ? 1000.0f / frameMs : 0.0f;

		if (ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen)) {
			float cr, cg, cb;
			FrameTimeColor(frameMs, cr, cg, cb);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(cr, cg, cb, 1.0f));
			ImGui::Text("Frame Time: %.3f ms  |  FPS: %.1f", frameMs, fps);
			ImGui::PopStyleColor();

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

		if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("RenderingStats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 80.0f);
				ImGui::TableHeadersRow();
#define ROW(label, fmt, ...)                                                                 \
	do {                                                                                       \
		ImGui::TableNextRow();                                                                 \
		ImGui::TableNextColumn();                                                              \
		ImGui::TextUnformatted(label);                                                         \
		ImGui::TableNextColumn();                                                              \
		ImGui::Text(fmt, __VA_ARGS__);                                                         \
	} while (0)
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

		if (ImGui::CollapsingHeader("Memory & Resources", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("MemoryStats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Resource", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableHeadersRow();
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("VRAM");
				ImGui::TableNextColumn();
				if (stats.VramBytes > 0)
					ImGui::Text("%.2f MB", stats.VramBytes / (1024.0 * 1024.0));
				else
					ImGui::TextUnformatted("N/A");
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Heap");
				ImGui::TableNextColumn();
				if (stats.HeapBytes > 0)
					ImGui::Text("%.2f MB", stats.HeapBytes / (1024.0 * 1024.0));
				else
					ImGui::TextUnformatted("N/A");
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Active Entities");
				ImGui::TableNextColumn();
				ImGui::Text("%u", stats.ActiveEntities);
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

	void RenderEditorSettingsPanel(bool& showOpen, bool& showPhysicsColliders, bool& showSelectionOutline) {
		if (!showOpen)
			return;
		if (!ImGui::Begin("Settings", &showOpen, ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		ImGui::Checkbox("Show physics colliders (2D)", &showPhysicsColliders);
		ImGui::Checkbox("Show selection outline", &showSelectionOutline);
		ImGui::End();
	}

} // namespace Ehu
