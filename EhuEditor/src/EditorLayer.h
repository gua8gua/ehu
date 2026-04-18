#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"

namespace Ehu {

	class KeyPressedEvent;

	class SceneViewportPanel;
	class ContentBrowserPanel;
	class ConsolePanel;
	class InspectorPanel;
	class HierarchyPanel;
	class ProjectEntryPanel;
	class EditorMenuBar;
	class EditorModalHost;
	class EditorSession;
	struct EditorUIState;

	/// 编辑器层：组合场景视口、资源预览、检视三面板，主菜单 File/Window，默认停靠布局
	class EditorLayer : public Layer {
	public:
		EditorLayer();
		~EditorLayer() override;

		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		bool OnEditorKeyPressed(KeyPressedEvent& e);
		void TryLoadFirstRecentProjectOnce();
		void ApplyDefaultDockLayoutOnce();
		void ShowError(const std::string& msg);

		SceneViewportPanel* m_SceneViewport = nullptr;
		ContentBrowserPanel* m_ContentBrowser = nullptr;
		ConsolePanel* m_Console = nullptr;
		InspectorPanel* m_Inspector = nullptr;
		HierarchyPanel* m_Hierarchy = nullptr;
		ProjectEntryPanel* m_ProjectEntry = nullptr;
		EditorMenuBar* m_MenuBar = nullptr;
		EditorModalHost* m_ModalHost = nullptr;
		EditorSession* m_Session = nullptr;
		EditorUIState* m_UIState = nullptr;

		bool m_DefaultLayoutApplied = false;
		bool m_FirstFrameTriedLoad = false;
	};

} // namespace Ehu
