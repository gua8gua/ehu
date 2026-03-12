#pragma once

#include "Core/Core.h"

namespace Ehu {

	/// 资源检视面板：根据 EditorContext 选中实体或资产绘制属性
	class InspectorPanel {
	public:
		InspectorPanel() = default;
		~InspectorPanel() = default;

		void OnImGuiRender();
	};

} // namespace Ehu
