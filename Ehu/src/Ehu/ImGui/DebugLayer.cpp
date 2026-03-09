#include "ehupch.h"
#include "DebugLayer.h"
#include "ImGuiWindowVisibility.h"
#include "Core/Application.h"
#include "Renderer/RenderQueue.h"
#include "imgui.h"

namespace Ehu {

	void DebugLayer::OnImGuiRender() {
		using namespace ImGuiWindowVisibility;
		if (!ShowStats) return;

		// 再次打开时回到默认停靠位置（从关闭变为打开时强制位置）
		{
			static bool s_WasVisible = false;
			bool justOpened = ShowStats && !s_WasVisible;
			if (justOpened) {
				float x, y;
				GetDefaultWindowPos("Stats", x, y);
				ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
			}
			s_WasVisible = ShowStats;
		}

		if (!ImGui::Begin("Stats", &ShowStats, ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		Application& app = Application::Get();
		float fps = app.GetFPS();
		float dt = app.GetDeltaTime();
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Delta: %.3f ms", dt * 1000.0f);

		const RenderQueue* queue = app.GetRenderQueue();
		if (queue) {
			const RenderStats& s = queue->GetLastFrameStats();
			ImGui::Separator();
			ImGui::Text("Draw Calls 2D: %u", s.DrawCalls2D);
			ImGui::Text("Draw Calls 3D: %u", s.DrawCalls3D);
			ImGui::Text("Triangles 2D: %u", s.Triangles2D);
			ImGui::Text("Triangles 3D: %u", s.Triangles3D);
		}
		ImGui::End();
	}

} // namespace Ehu
