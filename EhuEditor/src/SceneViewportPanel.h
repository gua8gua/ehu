#pragma once

#include "Core/Core.h"

namespace Ehu {

	class ViewportRenderer;

	/// 场景布置面板：ImGui 窗口内显示视口纹理，驱动 EditorCamera 输入
	class SceneViewportPanel {
	public:
		SceneViewportPanel();
		~SceneViewportPanel();

		void OnImGuiRender(class Application& app);

	private:
		ViewportRenderer* m_Viewport = nullptr;
		bool m_Focused = false;
		bool m_Hovered = false;
	};

} // namespace Ehu
