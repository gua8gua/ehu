#pragma once

namespace Ehu {

	class EditorSession;

	struct EditorWindowMenuState {
		bool* ShowSceneViewport = nullptr;
		bool* ShowHierarchy = nullptr;
		bool* ShowContentBrowser = nullptr;
		bool* ShowInspector = nullptr;
		bool* ShowStats = nullptr;
		bool* ShowDashboard = nullptr;
		bool* ShowConsole = nullptr;
		bool* ShowSettings = nullptr;
	};

	struct EditorMenuBarResult {
		bool RequestNewProject = false;
		bool RequestOpenProject = false;
		bool RequestSaveProject = false;
		bool RequestCloseProject = false;
		bool RequestNewScene = false;
		bool RequestOpenScene = false;
		bool RequestSaveScene = false;
		bool RequestSaveSceneAs = false;
		bool RequestReloadScripts = false;
		bool RequestTogglePlayMode = false;
		bool NextPlayMode = false;
		bool RequestToggleSimulateMode = false;
		bool NextSimulateMode = false;
	};

	class EditorMenuBar {
	public:
		EditorMenuBarResult OnImGuiRender(const EditorSession& session, const EditorWindowMenuState& windowState);
	};

} // namespace Ehu
