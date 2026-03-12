#pragma once

#include "Core/Layer.h"
#include <string>

namespace Ehu {

	class SceneViewportPanel;
	class ContentBrowserPanel;
	class InspectorPanel;

	/// 编辑器层：组合场景视口、资源预览、检视三面板，主菜单 File/Window，默认停靠布局
	class EditorLayer : public Layer {
	public:
		EditorLayer();
		~EditorLayer() override;

		void OnImGuiRender() override;

	private:
		void DrawMenuBar();
		void DrawFileMenu();
		void DrawWindowMenu();
		void DrawProjectEntryPanel();
		void TryLoadFirstRecentProjectOnce();
		void ApplyDefaultDockLayoutOnce();
		bool DoOpenProject(const std::string& projectFilePath);
		void DoCloseProject();

		SceneViewportPanel* m_SceneViewport = nullptr;
		ContentBrowserPanel* m_ContentBrowser = nullptr;
		InspectorPanel* m_Inspector = nullptr;

		bool m_ShowSceneViewport = true;
		bool m_ShowContentBrowser = true;
		bool m_ShowInspector = true;
		bool m_ShowHierarchy = true;

		bool m_DefaultLayoutApplied = false;
		bool m_FirstFrameTriedLoad = false;
		bool m_ShowNewProjectModal = false;
		bool m_ShowOpenProjectModal = false;
		char m_NewProjectDirBuf[512] = "";
		char m_NewProjectNameBuf[256] = "MyGame";
		char m_OpenProjectPathBuf[512] = "";
		bool m_ShowErrorPopup = false;
		std::string m_ErrorMessage;
		void ShowError(const std::string& msg);
	};

} // namespace Ehu
