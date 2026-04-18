#include "ProjectEntryPanel.h"
#include "EditorSession.h"
#include <imgui.h>
#include <vector>

namespace Ehu {

	ProjectEntryPanelResult ProjectEntryPanel::OnImGuiRender(const EditorSession& session) const {
		ProjectEntryPanelResult result;
		ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
		ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
		ImGui::SetNextWindowPos(workPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(workSize, ImGuiCond_Always);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
		if (!ImGui::Begin("ProjectEntry", nullptr, flags)) {
			ImGui::End();
			return result;
		}

		const float buttonW = 180.0f;
		ImGui::SetCursorPosY(workSize.y * 0.2f);
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		ImGui::Text("Ehu Editor");
		ImGui::Spacing();
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		if (ImGui::Button("新建项目", ImVec2(buttonW, 0)))
			result.RequestNewProject = true;
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		if (ImGui::Button("打开项目", ImVec2(buttonW, 0)))
			result.RequestOpenProject = true;
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("最近项目");
		std::vector<std::string> recent = session.GetRecentProjects();
		for (const std::string& recentPath : recent) {
			std::string label = recentPath.length() > 70 ? recentPath.substr(0, 67) + "..." : recentPath;
			if (ImGui::Selectable(label.c_str(), false))
				result.RecentProjectToOpen = recentPath;
		}
		ImGui::End();
		return result;
	}

} // namespace Ehu
