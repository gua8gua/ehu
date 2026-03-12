#include "ContentBrowserPanel.h"
#include "Editor/EditorContext.h"
#include "Project/Project.h"
#include "Project/AssetRegistry.h"
#include <imgui.h>

namespace Ehu {

	ContentBrowserPanel::ContentBrowserPanel() {
		m_Registry = new AssetRegistry();
	}

	ContentBrowserPanel::~ContentBrowserPanel() {
		delete m_Registry;
	}

	void ContentBrowserPanel::OnImGuiRender() {
		const bool visible = ImGui::Begin("Content Browser", nullptr);
		if (!visible) {
			ImGui::End();
			return;
		}

		Ref<Project> proj = Project::GetActive();
		if (!proj) {
			ImGui::TextUnformatted("未加载项目，请先创建或打开项目。");
			ImGui::End();
			return;
		}

		if (m_RefreshPending) {
			m_Registry->Refresh(*proj);
			m_RefreshPending = false;
		}

		const auto& entries = m_Registry->GetEntries();
		for (const AssetEntry& e : entries) {
			const char* icon = e.IsDirectory ? "[DIR]" : "[FILE]";
			if (ImGui::Selectable((std::string(icon) + " " + e.RelativePath).c_str(),
				EditorContext::Get().GetSelectedAsset() == e.RelativePath)) {
				if (!e.IsDirectory) {
					EditorContext::Get().ClearSelectedEntity();
					EditorContext::Get().SetSelectedAsset(e.RelativePath);
				}
			}
		}

		if (ImGui::Button("Refresh"))
			m_RefreshPending = true;

		ImGui::End();
	}

} // namespace Ehu
