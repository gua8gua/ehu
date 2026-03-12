#include "EditorLayer.h"
#include "SceneViewportPanel.h"
#include "ContentBrowserPanel.h"
#include "InspectorPanel.h"
#include "Core/Application.h"
#include "Editor/EditorContext.h"
#include "Editor/EditorPanelData.h"
#include "Project/Project.h"
#include "Platform/IO/FileDialog.h"
#include "ImGui/ImGuiLayer.h"
#include "ImGui/ImGuiWindowVisibility.h"
#include <imgui.h>
#include "imgui_internal.h"
#include <vector>
#include <cstring>

namespace Ehu {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		m_SceneViewport = new SceneViewportPanel();
		m_ContentBrowser = new ContentBrowserPanel();
		m_Inspector = new InspectorPanel();
	}

	EditorLayer::~EditorLayer() {
		delete m_SceneViewport;
		delete m_ContentBrowser;
		delete m_Inspector;
	}

	void EditorLayer::OnImGuiRender() {
		bool hasProject = Project::GetActive() != nullptr;

		if (!hasProject) {
			TryLoadFirstRecentProjectOnce();
			hasProject = Project::GetActive() != nullptr;
		}

		if (!hasProject) {
			DrawProjectEntryPanel();
		} else {
			DrawMenuBar();
			ApplyDefaultDockLayoutOnce();

			if (m_ShowSceneViewport)
				m_SceneViewport->OnImGuiRender(Application::Get());
			if (m_ShowHierarchy) {
				if (ImGui::Begin("Hierarchy", &m_ShowHierarchy)) {
					auto& provider = EditorPanelDataProvider::Get();
					HierarchySnapshot snapshot = provider.GetHierarchySnapshot();
					if (!snapshot.HasProject) {
						ImGui::TextUnformatted("无项目");
					} else if (snapshot.Entries.empty()) {
						const bool hasActivatedScenes = !Application::Get().GetActivatedScenes().empty();
						ImGui::TextUnformatted(hasActivatedScenes ? "当前场景无实体" : "项目无已激活场景，请先创建或加载场景");
					} else {
						for (const auto& entry : snapshot.Entries) {
							if (ImGui::Selectable(entry.Label.c_str(), entry.IsSelected))
								provider.SelectEntity(entry.Handle);
						}
					}
				}
				ImGui::End();
			}
			if (m_ShowContentBrowser)
				m_ContentBrowser->OnImGuiRender();
			if (m_ShowInspector)
				m_Inspector->OnImGuiRender();
		}

		// 新建/打开项目模态与错误提示（无项目时由进入面板触发，有项目时由菜单触发）
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
				if (dir.empty()) { ShowError("请选择项目目录"); }
				else if (name.empty()) { ShowError("请输入项目名称"); }
				else {
					EditorContext::Get().ClearAll();
					Application::Get().DeactivateAllScenes();
					Project::CloseActive();
					Ref<Project> proj = Project::New(dir, name);
					if (proj) {
						Application::Get().ActivateScenesFromProject(*proj);
						Project::AddToRecent(proj->GetProjectFilePath());
						ImGui::CloseCurrentPopup();
					} else
						ShowError("创建项目失败，请检查目录可写且路径有效");
				}
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
				if (path.empty()) { ShowError("请选择项目文件"); }
				else {
					if (DoOpenProject(path)) {
						ImGui::CloseCurrentPopup();
					}
					else
						ShowError("项目文件无效，无法加载");
				}
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
	}

	void EditorLayer::TryLoadFirstRecentProjectOnce() {
		if (m_FirstFrameTriedLoad)
			return;
		m_FirstFrameTriedLoad = true;
		std::vector<std::string> recent = Project::GetRecentProjects();
		if (recent.empty())
			return;
		DoOpenProject(recent[0]);
	}

	void EditorLayer::DrawProjectEntryPanel() {
		ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
		ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
		ImGui::SetNextWindowPos(workPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(workSize, ImGuiCond_Always);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
		if (!ImGui::Begin("ProjectEntry", nullptr, flags)) {
			ImGui::End();
			return;
		}
		const float buttonW = 180.0f;
		ImGui::SetCursorPosY(workSize.y * 0.2f);
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		ImGui::Text("Ehu Editor");
		ImGui::Spacing();
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		if (ImGui::Button("新建项目", ImVec2(buttonW, 0)))
			m_ShowNewProjectModal = true;
		ImGui::SetCursorPosX((workSize.x - buttonW) * 0.5f);
		if (ImGui::Button("打开项目", ImVec2(buttonW, 0)))
			m_ShowOpenProjectModal = true;
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("最近项目");
		std::vector<std::string> recent = Project::GetRecentProjects();
		for (size_t i = 0; i < recent.size(); ++i) {
			std::string label = recent[i].length() > 70 ? recent[i].substr(0, 67) + "..." : recent[i];
			if (ImGui::Selectable(label.c_str(), false)) {
				if (!DoOpenProject(recent[i]))
					ShowError("项目文件无效，无法加载");
			}
		}
		ImGui::End();
	}

	void EditorLayer::DrawMenuBar() {
		if (!ImGui::BeginMainMenuBar())
			return;
		DrawFileMenu();
		DrawWindowMenu();
		ImGui::EndMainMenuBar();
	}

	void EditorLayer::DrawFileMenu() {
		if (!ImGui::BeginMenu("File"))
			return;
		if (ImGui::MenuItem("New Project"))
			m_ShowNewProjectModal = true;
		if (ImGui::MenuItem("Open Project"))
			m_ShowOpenProjectModal = true;
		bool hasActive = Project::GetActive() != nullptr;
		if (ImGui::MenuItem("Save Project")) {
			if (!hasActive)
				ShowError("未选择项目");
			else
				Project::SaveActive();
		}
		if (ImGui::MenuItem("Close Project")) {
			if (!hasActive)
				ShowError("未选择项目");
			else
				DoCloseProject();
		}
		std::vector<std::string> recent = Project::GetRecentProjects();
		if (ImGui::BeginMenu("Recent", !recent.empty())) {
			for (const std::string& path : recent) {
				if (ImGui::MenuItem(path.c_str()) && !DoOpenProject(path))
					ShowError("项目文件无效，无法加载");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}

	void EditorLayer::DrawWindowMenu() {
		if (!ImGui::BeginMenu("Window"))
			return;
		ImGui::MenuItem("Scene", nullptr, &m_ShowSceneViewport);
		ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
		ImGui::MenuItem("Content Browser", nullptr, &m_ShowContentBrowser);
		ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
		ImGui::Separator();
		ImGui::MenuItem("Stats", nullptr, &ImGuiWindowVisibility::ShowStats);
		ImGui::MenuItem("Dashboard", "F3", &ImGuiWindowVisibility::ShowDashboard);
		ImGui::EndMenu();
	}

	void EditorLayer::ApplyDefaultDockLayoutOnce() {
		if (m_DefaultLayoutApplied)
			return;
		ImGuiLayer* imgui = Application::Get().GetImGuiLayer();
		ImGuiID root = imgui ? (ImGuiID)imgui->GetDockspaceRootId() : 0;
		if (root == 0)
			return;
		ImVec2 size = ImGui::GetMainViewport()->WorkSize;
		ImGui::DockBuilderSetNodeSize(root, size);
		ImGuiID center = root;
		ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
		ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);

		// 左侧：Hierarchy
		ImGui::DockBuilderDockWindow("Hierarchy", left);
		// 中央：Scene
		ImGui::DockBuilderDockWindow("Scene", center);
		// 右侧：Inspector + Content Browser
		ImGui::DockBuilderDockWindow("Inspector", right);
		ImGui::DockBuilderDockWindow("Content Browser", right);
		ImGui::DockBuilderFinish(root);
		m_DefaultLayoutApplied = true;
	}

	bool EditorLayer::DoOpenProject(const std::string& projectFilePath) {
		EditorContext::Get().ClearAll();
		Application::Get().DeactivateAllScenes();
		Project::CloseActive();
		Ref<Project> proj = Project::Load(projectFilePath);
		if (proj) {
			Application::Get().ActivateScenesFromProject(*proj);
			Project::AddToRecent(projectFilePath);
			return true;
		}
		return false;
	}

	void EditorLayer::DoCloseProject() {
		EditorContext::Get().ClearAll();
		Application::Get().DeactivateAllScenes();
		Project::CloseActive();
	}

	void EditorLayer::ShowError(const std::string& msg) {
		m_ErrorMessage = msg;
		m_ShowErrorPopup = true;
	}

} // namespace Ehu
