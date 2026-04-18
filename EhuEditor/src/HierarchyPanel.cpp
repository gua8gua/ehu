#include "HierarchyPanel.h"
#include "EditorSession.h"
#include "Editor/EditorPanelData.h"
#include <imgui.h>

namespace Ehu {

	bool HierarchyPanel::OnImGuiRender(const EditorSession& session, bool* pOpen) {
		if (pOpen && !*pOpen)
			return false;
		bool requestCreateEntity = false;
		const bool visible = ImGui::Begin("Hierarchy", pOpen);
		if (!visible) {
			ImGui::End();
			return requestCreateEntity;
		}

		auto& provider = EditorPanelDataProvider::Get();
		HierarchySnapshot snapshot = provider.GetHierarchySnapshot();
		if (ImGui::Button("Create Entity"))
			requestCreateEntity = true;

		if (!snapshot.HasProject) {
			ImGui::TextUnformatted("无项目");
		} else if (snapshot.Entries.empty()) {
			ImGui::TextUnformatted(session.HasActiveScene() ? "当前场景无实体" : "项目无已激活场景，请先创建或加载场景");
		} else {
			for (const auto& entry : snapshot.Entries) {
				if (ImGui::Selectable(entry.Label.c_str(), entry.IsSelected))
					provider.SelectEntity(entry.Handle);
			}
		}

		ImGui::End();
		return requestCreateEntity;
	}

} // namespace Ehu
