#include "SceneViewportPanel.h"
#include "Core/Application.h"
#include "Core/KeyCodes.h"
#include "Editor/ViewportRenderer.h"
#include "Renderer/Camera/EditorCamera.h"
#include "Platform/IO/Input.h"
#include <imgui.h>

namespace Ehu {

	SceneViewportPanel::SceneViewportPanel() {
		m_Viewport = new ViewportRenderer();
	}

	SceneViewportPanel::~SceneViewportPanel() {
		delete m_Viewport;
	}

	void SceneViewportPanel::OnImGuiRender(Application& app) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (!ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar)) {
			ImGui::End();
			ImGui::PopStyleVar();
			return;
		}
		ImGui::PopStyleVar();

		m_Focused = ImGui::IsWindowFocused();
		m_Hovered = ImGui::IsWindowHovered();

		ImVec2 size = ImGui::GetContentRegionAvail();
		if (size.x > 0 && size.y > 0) {
			m_Viewport->SetSize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
			m_Viewport->Render(app);

			uint32_t texId = m_Viewport->GetColorAttachmentTextureID(0);
			ImGui::Image((ImTextureID)(uintptr_t)texId, size, ImVec2(0, 1), ImVec2(1, 0));

			if (m_Focused && m_Viewport->GetEditorCamera()) {
				EditorCamera* cam = m_Viewport->GetEditorCamera();
				float delta = 0.05f;
				if (Input::IsKeyPressed(Key::Left))
					cam->SetRotationDelta(0.0f, delta);
				if (Input::IsKeyPressed(Key::Right))
					cam->SetRotationDelta(0.0f, -delta);
				if (Input::IsKeyPressed(Key::Up))
					cam->SetRotationDelta(-delta, 0.0f);
				if (Input::IsKeyPressed(Key::Down))
					cam->SetRotationDelta(delta, 0.0f);
				cam->OnUpdate();
			}
		}

		ImGui::End();
	}

} // namespace Ehu
