#pragma once

namespace Ehu {

	/// 浮动面板默认位置（相对主视口 WorkArea），与旧 ImGuiWindowVisibility::GetDefaultWindowPos 一致
	void GetDefaultEditorPanelPos(const char* windowName, float& outX, float& outY);

	void RenderEditorStatsPanel(bool& showOpen);
	void RenderEditorDashboardPanel(bool& showOpen);
	void RenderEditorSettingsPanel(bool& showOpen, bool& showPhysicsColliders, bool& showSelectionOutline);

} // namespace Ehu
