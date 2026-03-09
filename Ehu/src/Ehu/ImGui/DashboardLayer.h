#pragma once

#include "Core/Layer.h"

namespace Ehu {

	/// 仪表盘 Layer：只读 DashboardStats，分块展示 Timing / Rendering / Memory，含折线图
	class EHU_API DashboardLayer : public Layer {
	public:
		DashboardLayer() : Layer("DashboardLayer") {}
		void OnImGuiRender() override;
	};

} // namespace Ehu
