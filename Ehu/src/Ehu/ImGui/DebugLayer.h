#pragma once

#include "Core/Layer.h"

namespace Ehu {

	/// 调试/统计面板：在 OnImGuiRender 中显示 FPS、Draw Call 数、三角形数等
	class EHU_API DebugLayer : public Layer {
	public:
		DebugLayer() : Layer("DebugLayer") {}
		void OnImGuiRender() override;
	};

} // namespace Ehu
