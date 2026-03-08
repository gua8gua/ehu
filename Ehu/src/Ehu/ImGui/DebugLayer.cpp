#include "ehupch.h"
#include "DebugLayer.h"
#include "Core/Application.h"
#include "Renderer/RenderQueue.h"
#include "imgui.h"

namespace Ehu {

	void DebugLayer::OnImGuiRender() {
		ImGui::Begin("Stats");
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
