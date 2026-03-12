#pragma once

#include "Core/Core.h"

namespace Ehu {

	/// 各调试/仪表盘窗口的显示状态，由顶部菜单栏与各 Layer 共享；再次打开时使用默认位置
	namespace ImGuiWindowVisibility {
		extern EHU_API bool ShowStats;
		extern EHU_API bool ShowDashboard;

		/// 默认停靠位置（相对主视口 WorkArea）：Stats 左上、Dashboard 其右侧
		void EHU_API GetDefaultWindowPos(const char* windowName, float& outX, float& outY);
	}

} // namespace Ehu
