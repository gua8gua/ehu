#include "ImGuiWindowVisibility.h"
#include "imgui.h"
#include <cstring>

namespace Ehu {

	namespace ImGuiWindowVisibility {
		bool ShowStats = true;
		bool ShowDashboard = true;

		void GetDefaultWindowPos(const char* windowName, float& outX, float& outY) {
			ImGuiViewport* vp = ImGui::GetMainViewport();
			const float menuBarHeight = 21.0f; // 与 BeginMainMenuBar 高度一致
			const float pad = 20.0f;
			const float startY = vp->WorkPos.y + menuBarHeight + pad;

			if (strcmp(windowName, "Stats") == 0) {
				outX = vp->WorkPos.x + pad;
				outY = startY;
			} else if (strcmp(windowName, "Dashboard") == 0) {
				outX = vp->WorkPos.x + pad + 280.0f; // Stats 右侧
				outY = startY;
			} else {
				outX = vp->WorkPos.x + pad;
				outY = startY;
			}
		}
	}

} // namespace Ehu
