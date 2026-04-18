#include "ConsolePanel.h"
#include "EditorAuxPanels.h"
#include "Core/Log.h"
#include <imgui.h>

namespace Ehu {

	void ConsolePanel::OnImGuiRender(bool* pOpen) {
		if (!pOpen || !*pOpen)
			return;

		static bool s_WasVisible = false;
		const bool justOpened = *pOpen && !s_WasVisible;
		if (justOpened) {
			float x, y;
			GetDefaultEditorPanelPos("Console", x, y);
			ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
		}
		s_WasVisible = *pOpen;

		if (!ImGui::Begin("Console", pOpen)) {
			ImGui::End();
			return;
		}

		if (ImGui::Button("Clear"))
			Log::ClearBufferedMessages();
		ImGui::Separator();

		const auto& messages = Log::GetBufferedMessages();
		if (ImGui::BeginChild("ConsoleScroll")) {
			for (const BufferedLogMessage& message : messages) {
				ImGui::Text("[%s] %s: %s", message.Level.c_str(), message.Logger.c_str(), message.Message.c_str());
			}
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();

		ImGui::End();
	}

} // namespace Ehu
