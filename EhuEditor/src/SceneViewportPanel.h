#pragma once

#include "Core/Core.h"
#include <string>

	namespace Ehu {

	class ViewportRenderer;
	class EditorSession;
	struct EditorUIState;
	class Texture2D;

	struct SceneViewportPanelResult {
		bool RequestPlay = false;
		bool RequestSimulate = false;
		bool RequestStop = false;
		bool RequestPauseToggle = false;
		bool RequestStep = false;
		std::string OpenSceneRelativePath;
	};

	/// 场景视口：FBO + EditorCamera，工具栏驱动 Play/Simulate/Stop，编辑模式下拾取与简易 gizmo
	class SceneViewportPanel {
	public:
		SceneViewportPanel();
		~SceneViewportPanel();

		SceneViewportPanelResult OnImGuiRender(EditorSession& session, bool* pOpen, EditorUIState* uiState);

	private:
		enum class GizmoMode {
			None = 0,
			Translate,
			Rotate,
			Scale
		};

		void DrawToolbar(const EditorSession& session, SceneViewportPanelResult& result);
		void EnsureToolbarIcons();
		void ReleaseToolbarIcons();

		ViewportRenderer* m_Viewport = nullptr;
		Texture2D* m_IconPlay = nullptr;
		Texture2D* m_IconPause = nullptr;
		Texture2D* m_IconSimulate = nullptr;
		Texture2D* m_IconStep = nullptr;
		Texture2D* m_IconStop = nullptr;
		bool m_ToolbarIconsReady = false;
		bool m_Focused = false;
		bool m_Hovered = false;
		GizmoMode m_GizmoMode = GizmoMode::Translate;
		bool m_GizmoActive = false;
		float m_LastMouseX = 0.0f;
		float m_LastMouseY = 0.0f;
		float m_LastRotateAngle = 0.0f;
	};

} // namespace Ehu
