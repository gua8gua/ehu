#include "EditorModalHost.h"
#include "Core/Application.h"
#include "Editor/CreationRegistry.h"
#include "Editor/EditorContext.h"
#include "ECS/LayerRegistry.h"
#include "Platform/IO/FileDialog.h"
#include <imgui.h>
#include <cstring>
#include <vector>

namespace Ehu {

	void EditorModalHost::ShowError(const std::string& message) {
		m_ErrorMessage = message;
		m_ShowErrorPopup = true;
	}

	void EditorModalHost::OnImGuiRender(
		const CreateProjectHandler& createProject,
		const OpenProjectHandler& openProject,
		const CreateEntityHandler& createEntity,
		Scene* preferredEntityTargetScene) {
		if (m_ShowNewProjectModal) {
			ImGui::OpenPopup("New Project");
			m_ShowNewProjectModal = false;
		}
		if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputText("Project directory", m_NewProjectDirBuf, sizeof(m_NewProjectDirBuf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("Browse...")) {
				std::string chosen;
				if (FileDialog::OpenFolder("Select project folder", chosen) && chosen.size() < sizeof(m_NewProjectDirBuf))
					strncpy(m_NewProjectDirBuf, chosen.c_str(), sizeof(m_NewProjectDirBuf) - 1), m_NewProjectDirBuf[sizeof(m_NewProjectDirBuf) - 1] = '\0';
			}
			ImGui::InputText("Project name", m_NewProjectNameBuf, sizeof(m_NewProjectNameBuf));
			if (ImGui::Button("Create")) {
				std::string dir(m_NewProjectDirBuf);
				std::string name(m_NewProjectNameBuf);
				if (dir.empty())
					ShowError("请选择项目目录");
				else if (name.empty())
					ShowError("请输入项目名称");
				else if (createProject && createProject(NewProjectRequest{ dir, name }))
					ImGui::CloseCurrentPopup();
				else
					ShowError("创建项目失败，请检查目录可写且路径有效");
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (m_ShowOpenProjectModal) {
			ImGui::OpenPopup("Open Project");
			m_ShowOpenProjectModal = false;
		}
		if (ImGui::BeginPopupModal("Open Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputText("Project file (.ehuproject)", m_OpenProjectPathBuf, sizeof(m_OpenProjectPathBuf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("Browse...")) {
				std::string chosen;
				if (FileDialog::OpenFile("Open Ehu Project", "Ehu Project (*.ehuproject)", "*.ehuproject", chosen) && chosen.size() < sizeof(m_OpenProjectPathBuf))
					strncpy(m_OpenProjectPathBuf, chosen.c_str(), sizeof(m_OpenProjectPathBuf) - 1), m_OpenProjectPathBuf[sizeof(m_OpenProjectPathBuf) - 1] = '\0';
			}
			if (ImGui::Button("Open")) {
				std::string path(m_OpenProjectPathBuf);
				if (path.empty())
					ShowError("请选择项目文件");
				else if (openProject && openProject(path))
					ImGui::CloseCurrentPopup();
				else
					ShowError("项目文件无效，无法加载");
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (m_ShowErrorPopup) {
			ImGui::OpenPopup("提示");
			m_ShowErrorPopup = false;
		}
		if (ImGui::BeginPopupModal("提示", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted(m_ErrorMessage.c_str());
			if (ImGui::Button("确定")) {
				m_ErrorMessage.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (m_ShowCreateEntityPopup) {
			ImGui::OpenPopup("Create Entity");
			m_ShowCreateEntityPopup = false;
		}
		if (ImGui::BeginPopupModal("Create Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			auto creators = CreationRegistry::GetEntityCreators();
			auto renderChannels = LayerRegistry::GetRenderChannels();
			auto collisionLayers = LayerRegistry::GetCollisionLayers();
			if (creators.empty() || renderChannels.empty() || collisionLayers.empty()) {
				ImGui::TextUnformatted("创建器或层注册表为空。");
			} else {
				if (m_SelectedEntityCreator < 0 || m_SelectedEntityCreator >= static_cast<int>(creators.size()))
					m_SelectedEntityCreator = 0;
				if (m_SelectedRenderChannel < 0 || m_SelectedRenderChannel >= static_cast<int>(renderChannels.size()))
					m_SelectedRenderChannel = 0;
				if (m_SelectedCollisionLayer < 0 || m_SelectedCollisionLayer >= static_cast<int>(collisionLayers.size()))
					m_SelectedCollisionLayer = 0;

				std::vector<const char*> creatorLabels;
				creatorLabels.reserve(creators.size());
				for (const auto& creator : creators)
					creatorLabels.push_back(creator.DisplayName.c_str());
				ImGui::Combo("Preset", &m_SelectedEntityCreator, creatorLabels.data(), static_cast<int>(creatorLabels.size()));
				ImGui::InputText("Name", m_NewEntityNameBuf, sizeof(m_NewEntityNameBuf));

				std::vector<const char*> channelLabels;
				channelLabels.reserve(renderChannels.size());
				for (const auto& channel : renderChannels)
					channelLabels.push_back(channel.second.c_str());
				ImGui::Combo("RenderChannel", &m_SelectedRenderChannel, channelLabels.data(), static_cast<int>(channelLabels.size()));
				ImGui::InputText("New RenderChannel", m_NewRenderChannelNameBuf, sizeof(m_NewRenderChannelNameBuf));
				ImGui::SameLine();
				if (ImGui::Button("Register RenderChannel") && m_NewRenderChannelNameBuf[0] != '\0') {
					LayerRegistry::RegisterRenderChannel(m_NewRenderChannelNameBuf);
					m_NewRenderChannelNameBuf[0] = '\0';
				}

				std::vector<const char*> collisionLabels;
				collisionLabels.reserve(collisionLayers.size());
				for (const auto& collisionLayer : collisionLayers)
					collisionLabels.push_back(collisionLayer.second.c_str());
				ImGui::Combo("CollisionLayer", &m_SelectedCollisionLayer, collisionLabels.data(), static_cast<int>(collisionLabels.size()));
				ImGui::InputText("New CollisionLayer", m_NewCollisionLayerNameBuf, sizeof(m_NewCollisionLayerNameBuf));
				ImGui::SameLine();
				if (ImGui::Button("Register CollisionLayer") && m_NewCollisionLayerNameBuf[0] != '\0') {
					LayerRegistry::RegisterCollisionLayer(m_NewCollisionLayerNameBuf);
					m_NewCollisionLayerNameBuf[0] = '\0';
				}

				if (ImGui::Button("Create")) {
					Scene* targetScene = preferredEntityTargetScene;
					EditorContext& context = EditorContext::Get();
					if (!targetScene && context.HasActiveScene())
						targetScene = context.GetActiveScene();
					if (!targetScene && context.HasEntitySelection())
						targetScene = context.GetSelectedScene();
					if (!targetScene) {
						const auto& scenes = Application::Get().GetActivatedScenes();
						targetScene = scenes.empty() ? nullptr : scenes.front();
					}
					if (!targetScene) {
						ShowError("无可用场景，无法创建实体");
					} else {
						const uint32_t collisionBit = collisionLayers[static_cast<size_t>(m_SelectedCollisionLayer)].first;
						CreateEntityRequest request;
						request.CreatorId = creators[static_cast<size_t>(m_SelectedEntityCreator)].Id;
						request.TargetScene = targetScene;
						request.Name = m_NewEntityNameBuf;
						request.RenderChannel = renderChannels[static_cast<size_t>(m_SelectedRenderChannel)].first;
						request.CollisionLayer = collisionBit;
						request.CollisionMask = LayerRegistry::GetCollisionDefaultMask(collisionBit);
						if (createEntity && createEntity(request)) {
							m_NewEntityNameBuf[0] = '\0';
							ImGui::CloseCurrentPopup();
						} else {
							ShowError("实体创建失败");
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

} // namespace Ehu
