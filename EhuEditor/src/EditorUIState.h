#pragma once

namespace Ehu {

	struct EditorUIState {
		bool ShowSceneViewport = true;
		bool ShowContentBrowser = true;
		bool ShowConsole = true;
		bool ShowInspector = true;
		bool ShowHierarchy = true;
		bool ShowStats = true;
		bool ShowDashboard = true;
		bool ShowSettings = false;
		/// 视口叠加：2D 物理碰撞体线框（编辑/模拟/运行态均按当前活动场景绘制）
		bool ShowPhysicsColliders = false;
		/// 视口叠加：选中实体矩形描边
		bool ShowSelectionOutline = true;
	};

} // namespace Ehu
