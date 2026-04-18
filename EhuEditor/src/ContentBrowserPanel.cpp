#include "ContentBrowserPanel.h"
#include "EditorSession.h"
#include "Editor/EditorContext.h"
#include "Editor/CreationRegistry.h"
#include "Project/Project.h"
#include "Project/AssetRegistry.h"
#include "Core/FileSystem.h"
#include <imgui.h>
#include <vector>

namespace Ehu {

	ContentBrowserPanel::ContentBrowserPanel() {
		m_Registry = new AssetRegistry();
	}

	ContentBrowserPanel::~ContentBrowserPanel() {
		delete m_Registry;
	}

	ContentBrowserPanelResult ContentBrowserPanel::OnImGuiRender(const EditorSession& session, bool* pOpen) {
		ContentBrowserPanelResult out;
		if (pOpen && !*pOpen)
			return out;
		const bool visible = ImGui::Begin("Content Browser", pOpen);
		if (!visible) {
			ImGui::End();
			return out;
		}

		Ref<Project> proj = session.GetProject();
		if (!proj) {
			ImGui::TextUnformatted("未加载项目，请先创建或打开项目。");
			ImGui::End();
			return out;
		}

		if (m_RefreshPending) {
			m_Registry->Refresh(*proj);
			m_RefreshPending = false;
			if (!m_CurrentDirectory.empty()) {
				bool directoryStillExists = false;
				for (const AssetEntry& entry : m_Registry->GetEntries()) {
					if (entry.IsDirectory && entry.RelativePath == m_CurrentDirectory) {
						directoryStillExists = true;
						break;
					}
				}
				if (!directoryStillExists)
					m_CurrentDirectory.clear();
			}
		}

		const auto& entries = m_Registry->GetEntries();
		if (ImGui::Button("Assets"))
			m_CurrentDirectory.clear();
		if (!m_CurrentDirectory.empty()) {
			ImGui::SameLine();
			ImGui::Text("Current: %s", m_CurrentDirectory.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Up"))
				m_CurrentDirectory = FileSystem::GetParentPath(m_CurrentDirectory);
		}
		ImGui::SameLine();
		if (ImGui::Button("Create"))
			m_ShowCreatePopup = true;
		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
			m_RefreshPending = true;
		ImGui::Separator();

		for (const AssetEntry& e : entries) {
			const std::string parentDirectory = FileSystem::GetParentPath(e.RelativePath);
			if (parentDirectory != m_CurrentDirectory)
				continue;
			const char* icon = e.IsDirectory ? "[DIR]" : "[FILE]";
			const std::string label = e.IsDirectory ? (std::string(icon) + " " + FileSystem::GetFileName(e.RelativePath) + "/")
				: (std::string(icon) + " " + FileSystem::GetFileName(e.RelativePath));
			if (ImGui::Selectable(label.c_str(),
				EditorContext::Get().GetSelectedAsset() == e.RelativePath)) {
				EditorContext::Get().ClearSelectedEntity();
				if (e.IsDirectory)
					m_CurrentDirectory = e.RelativePath;
				else
					EditorContext::Get().SetSelectedAsset(e.RelativePath);
			}
			if (!e.IsDirectory && FileSystem::GetExtension(e.RelativePath) == ".ehuscene"
				&& ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				out.OpenSceneRelativePath = e.RelativePath;

			if (!e.IsDirectory && ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ASSET", e.RelativePath.c_str(), e.RelativePath.size() + 1);
				ImGui::TextUnformatted(e.RelativePath.c_str());
				ImGui::EndDragDropSource();
			}
		}

		if (m_ShowCreatePopup) {
			ImGui::OpenPopup("Create Asset");
			m_ShowCreatePopup = false;
		}

		if (ImGui::BeginPopupModal("Create Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			auto creators = CreationRegistry::GetAssetCreators();
			if (creators.empty()) {
				ImGui::TextUnformatted("无可用创建器");
			} else {
				if (m_SelectedCreator < 0 || m_SelectedCreator >= static_cast<int>(creators.size()))
					m_SelectedCreator = 0;
				std::vector<const char*> labels;
				labels.reserve(creators.size());
				for (const auto& creator : creators)
					labels.push_back(creator.DisplayName.c_str());
				ImGui::Combo("Type", &m_SelectedCreator, labels.data(), static_cast<int>(labels.size()));
				ImGui::InputText("Name", m_NewAssetNameBuf, sizeof(m_NewAssetNameBuf));
				const std::string selectedAsset = EditorContext::Get().GetSelectedAsset();
				std::string targetDirectory = m_CurrentDirectory;
				if (!selectedAsset.empty()) {
					AssetEntry matched{};
					bool found = false;
					for (const AssetEntry& e : entries) {
						if (e.RelativePath == selectedAsset) {
							matched = e;
							found = true;
							break;
						}
					}
					if (found)
						targetDirectory = matched.IsDirectory ? matched.RelativePath : FileSystem::GetParentPath(matched.RelativePath);
				}

				ImGui::Text("Target: %s", targetDirectory.empty() ? "<Assets Root>" : targetDirectory.c_str());
				if (ImGui::Button("Create")) {
					AssetCreateContext context{ *proj, targetDirectory, m_NewAssetNameBuf, {} };
					if (CreationRegistry::CreateAssetById(creators[static_cast<size_t>(m_SelectedCreator)].Id, context)) {
						m_RefreshPending = true;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
		return out;
	}

} // namespace Ehu
