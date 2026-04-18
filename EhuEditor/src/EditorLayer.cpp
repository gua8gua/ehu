#include "EditorLayer.h"
#include "SceneViewportPanel.h"
#include "ContentBrowserPanel.h"
#include "ConsolePanel.h"
#include "InspectorPanel.h"
#include "HierarchyPanel.h"
#include "ProjectEntryPanel.h"
#include "EditorMenuBar.h"
#include "EditorModalHost.h"
#include "EditorSession.h"
#include "EditorUIState.h"
#include "Core/Application.h"
#include "Editor/EditorContext.h"
#include "Editor/EditorPanelData.h"
#include "Project/Project.h"
#include "Scene/Scene.h"
#include "EditorAuxPanels.h"
#include "ImGui/ImGuiLayer.h"
#include "Platform/IO/Input.h"
#include "Core/KeyCodes.h"
#include "Editor/EditorContext.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "ECS/Entity.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace Ehu {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		m_SceneViewport = new SceneViewportPanel();
		m_ContentBrowser = new ContentBrowserPanel();
		m_Console = new ConsolePanel();
		m_Inspector = new InspectorPanel();
		m_Hierarchy = new HierarchyPanel();
		m_ProjectEntry = new ProjectEntryPanel();
		m_MenuBar = new EditorMenuBar();
		m_ModalHost = new EditorModalHost();
		m_Session = new EditorSession();
		m_UIState = new EditorUIState();
		Application::Get().SetPlayMode(false);
	}

	EditorLayer::~EditorLayer() {
		delete m_SceneViewport;
		delete m_ContentBrowser;
		delete m_Console;
		delete m_Inspector;
		delete m_Hierarchy;
		delete m_ProjectEntry;
		delete m_MenuBar;
		delete m_ModalHost;
		delete m_Session;
		delete m_UIState;
	}

	void EditorLayer::OnImGuiRender() {
		bool hasProject = m_Session->HasProject();

		if (!hasProject) {
			TryLoadFirstRecentProjectOnce();
			hasProject = m_Session->HasProject();
		}

		if (!hasProject) {
			ProjectEntryPanelResult entryResult = m_ProjectEntry->OnImGuiRender(*m_Session);
			if (entryResult.RequestNewProject)
				m_ModalHost->RequestNewProject();
			if (entryResult.RequestOpenProject)
				m_ModalHost->RequestOpenProject();
			if (!entryResult.RecentProjectToOpen.empty() && !m_Session->OpenProject(entryResult.RecentProjectToOpen))
				ShowError(m_Session->GetLastError());
		} else {
			if (ImGuiLayer* imgui = Application::Get().GetImGuiLayer())
				imgui->BlockEvents(true);

			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
				m_Session->NewScene();
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
				m_Session->OpenSceneDialog();
			if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S, false))
				m_Session->SaveSceneAs();
			else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
				m_Session->SaveScene();
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false))
				m_Session->ReloadScriptAssemblies();

			{
				static bool s_WasF3 = false;
				const bool f3 = Input::IsKeyPressed(Key::F3);
				if (f3 && !s_WasF3)
					m_UIState->ShowDashboard = !m_UIState->ShowDashboard;
				s_WasF3 = f3;
			}

			EditorWindowMenuState windowState{
				&m_UIState->ShowSceneViewport,
				&m_UIState->ShowHierarchy,
				&m_UIState->ShowContentBrowser,
				&m_UIState->ShowInspector,
				&m_UIState->ShowStats,
				&m_UIState->ShowDashboard,
				&m_UIState->ShowConsole,
				&m_UIState->ShowSettings
			};
			EditorMenuBarResult menuResult = m_MenuBar->OnImGuiRender(*m_Session, windowState);
			if (menuResult.RequestNewProject)
				m_ModalHost->RequestNewProject();
			if (menuResult.RequestOpenProject)
				m_ModalHost->RequestOpenProject();
			if (menuResult.RequestSaveProject) {
				if (!m_Session->HasProject())
					ShowError("未选择项目");
				else
					Project::SaveActive();
			}
			if (menuResult.RequestNewScene && !m_Session->NewScene())
				ShowError(m_Session->GetLastError());
			if (menuResult.RequestOpenScene && !m_Session->OpenSceneDialog())
				ShowError(m_Session->GetLastError());
			if (menuResult.RequestSaveScene && !m_Session->SaveScene())
				ShowError(m_Session->GetLastError());
			if (menuResult.RequestSaveSceneAs && !m_Session->SaveSceneAs())
				ShowError(m_Session->GetLastError());
			if (menuResult.RequestReloadScripts && !m_Session->ReloadScriptAssemblies())
				ShowError(m_Session->GetLastError());
			if (menuResult.RequestTogglePlayMode) {
				if (menuResult.NextPlayMode && !m_Session->BeginPlayMode())
					ShowError(m_Session->GetLastError());
				else if (!menuResult.NextPlayMode)
					m_Session->StopScene();
			}
			if (menuResult.RequestToggleSimulateMode) {
				if (menuResult.NextSimulateMode && !m_Session->BeginSimulationMode())
					ShowError(m_Session->GetLastError());
				else if (!menuResult.NextSimulateMode)
					m_Session->StopScene();
			}
			if (menuResult.RequestCloseProject) {
				if (!m_Session->HasProject())
					ShowError("未选择项目");
				else
					m_Session->CloseProject();
			}
			ApplyDefaultDockLayoutOnce();

			{
				SceneViewportPanelResult viewportResult = m_SceneViewport->OnImGuiRender(*m_Session, &m_UIState->ShowSceneViewport, m_UIState);
				if (viewportResult.RequestPlay && !m_Session->BeginPlayMode())
					ShowError(m_Session->GetLastError());
				if (viewportResult.RequestSimulate && !m_Session->BeginSimulationMode())
					ShowError(m_Session->GetLastError());
				if (viewportResult.RequestStop)
					m_Session->StopScene();
				if (viewportResult.RequestPauseToggle) {
					if (Scene* s = m_Session->GetActiveScene())
						s->SetPaused(!s->IsPaused());
				}
				if (viewportResult.RequestStep) {
					if (Scene* s = m_Session->GetActiveScene())
						s->Step(1);
				}
				if (!viewportResult.OpenSceneRelativePath.empty() && !m_Session->OpenScene(viewportResult.OpenSceneRelativePath))
					ShowError(m_Session->GetLastError());
			}
			if (m_Hierarchy->OnImGuiRender(*m_Session, &m_UIState->ShowHierarchy))
				m_ModalHost->RequestCreateEntity();
			{
				ContentBrowserPanelResult browserResult = m_ContentBrowser->OnImGuiRender(*m_Session, &m_UIState->ShowContentBrowser);
				if (!browserResult.OpenSceneRelativePath.empty() && !m_Session->OpenScene(browserResult.OpenSceneRelativePath))
					ShowError(m_Session->GetLastError());
			}
			m_Console->OnImGuiRender(&m_UIState->ShowConsole);
			m_Inspector->OnImGuiRender(*m_Session, &m_UIState->ShowInspector);
			RenderEditorStatsPanel(m_UIState->ShowStats);
			RenderEditorDashboardPanel(m_UIState->ShowDashboard);
			RenderEditorSettingsPanel(m_UIState->ShowSettings, m_UIState->ShowPhysicsColliders, m_UIState->ShowSelectionOutline);
		}

		m_ModalHost->OnImGuiRender(
			[this](const NewProjectRequest& request) {
				return m_Session->CreateProject(request.Directory, request.Name);
			},
			[this](const std::string& projectFilePath) {
				return m_Session->OpenProject(projectFilePath);
			},
			[](const CreateEntityRequest& request) {
				auto& provider = EditorPanelDataProvider::Get();
				return provider.CreateEntity(
					request.CreatorId,
					request.TargetScene,
					request.Name,
					request.RenderChannel,
					request.CollisionLayer,
					request.CollisionMask);
			},
			m_Session->GetEntityCreationTargetScene());
	}

	void EditorLayer::TryLoadFirstRecentProjectOnce() {
		if (m_FirstFrameTriedLoad)
			return;
		m_FirstFrameTriedLoad = true;
		m_Session->TryOpenFirstRecentProject();
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
		ImGui::DockBuilderDockWindow("Settings", right);
		ImGui::DockBuilderFinish(root);
		m_DefaultLayoutApplied = true;
	}

	void EditorLayer::ShowError(const std::string& msg) {
		m_ModalHost->ShowError(msg);
	}

	void EditorLayer::OnEvent(Event& e) {
		if (!m_Session || !m_Session->HasProject())
			return;
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& ke) {
			return OnEditorKeyPressed(ke);
		});
	}

	bool EditorLayer::OnEditorKeyPressed(KeyPressedEvent& e) {
		if (e.IsRepeat())
			return false;
		if (!m_Session->IsEditing())
			return false;
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantTextInput)
			return false;

		Scene* scene = m_Session->GetEditorScene();
		if (!scene)
			return false;

		const bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);

		if (e.GetKeyCode() == Key::D && ctrl) {
			if (!EditorContext::Get().HasEntitySelection() || EditorContext::Get().GetSelectedScene() != scene)
				return false;
			Entity sel = EditorContext::Get().GetSelectedEntity();
			if (!scene->GetWorld().IsValid(sel))
				return false;
			Entity neu = scene->DuplicateEntity(sel);
			EditorContext::Get().SetSelectedEntity(scene, neu);
			return true;
		}

		if (e.GetKeyCode() == Key::Delete) {
			if (ImGui::IsAnyItemActive())
				return false;
			ImGuiLayer* imgui = Application::Get().GetImGuiLayer();
			if (imgui && imgui->GetActiveWidgetID() != 0)
				return false;
			if (!EditorContext::Get().HasEntitySelection() || EditorContext::Get().GetSelectedScene() != scene)
				return false;
			Entity sel = EditorContext::Get().GetSelectedEntity();
			if (!scene->GetWorld().IsValid(sel))
				return false;
			scene->DestroyEntity(sel);
			EditorContext::Get().ClearSelectedEntity();
			return true;
		}

		return false;
	}

} // namespace Ehu
