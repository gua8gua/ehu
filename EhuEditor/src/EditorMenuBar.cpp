#include "EditorMenuBar.h"
#include "EditorSession.h"
#include <imgui.h>

namespace Ehu {

	EditorMenuBarResult EditorMenuBar::OnImGuiRender(const EditorSession& session, const EditorWindowMenuState& windowState) {
		EditorMenuBarResult result;
		if (!ImGui::BeginMainMenuBar())
			return result;

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project"))
				result.RequestNewProject = true;
			if (ImGui::MenuItem("Open Project"))
				result.RequestOpenProject = true;
			ImGui::Separator();
			if (ImGui::MenuItem("New Scene"))
				result.RequestNewScene = true;
			if (ImGui::MenuItem("Open Scene"))
				result.RequestOpenScene = true;
			if (ImGui::MenuItem("Save Scene"))
				result.RequestSaveScene = true;
			if (ImGui::MenuItem("Save Scene As"))
				result.RequestSaveSceneAs = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Save Project"))
				result.RequestSaveProject = true;
			if (ImGui::MenuItem("Close Project"))
				result.RequestCloseProject = true;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Script")) {
			if (ImGui::MenuItem("Reload Assemblies", "Ctrl+R"))
				result.RequestReloadScripts = true;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			const bool playing = session.IsPlaying();
			const bool simulating = session.IsSimulating();
			if (ImGui::MenuItem("Play Mode", nullptr, playing)) {
				result.RequestTogglePlayMode = true;
				result.NextPlayMode = !playing;
			}
			if (ImGui::MenuItem("Simulate Mode", nullptr, simulating)) {
				result.RequestToggleSimulateMode = true;
				result.NextSimulateMode = !simulating;
			}

			ImGui::Separator();
			// 仅判空指针；勿用 *pOpen 作为条件，否则面板关闭后菜单项消失无法重开
			if (windowState.ShowSceneViewport != nullptr)
				ImGui::MenuItem("Scene", nullptr, windowState.ShowSceneViewport);
			if (windowState.ShowHierarchy != nullptr)
				ImGui::MenuItem("Hierarchy", nullptr, windowState.ShowHierarchy);
			if (windowState.ShowContentBrowser != nullptr)
				ImGui::MenuItem("Content Browser", nullptr, windowState.ShowContentBrowser);
			if (windowState.ShowInspector != nullptr)
				ImGui::MenuItem("Inspector", nullptr, windowState.ShowInspector);

			ImGui::Separator();
			if (windowState.ShowStats != nullptr)
				ImGui::MenuItem("Stats", nullptr, windowState.ShowStats);
			if (windowState.ShowDashboard != nullptr)
				ImGui::MenuItem("Dashboard", "F3", windowState.ShowDashboard);
			if (windowState.ShowConsole != nullptr)
				ImGui::MenuItem("Console", nullptr, windowState.ShowConsole);
			if (windowState.ShowSettings != nullptr)
				ImGui::MenuItem("Settings", nullptr, windowState.ShowSettings);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
		return result;
	}

} // namespace Ehu
