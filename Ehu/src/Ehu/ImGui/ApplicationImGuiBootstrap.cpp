#include "ehupch.h"
#include "Core/Application.h"
#include "ImGuiLayer.h"

namespace Ehu {

	namespace {

		void BeginImGuiFrame(ImGuiLayer* layer) {
			if (layer)
				layer->Begin();
		}

		void EndImGuiFrame(ImGuiLayer* layer) {
			if (layer)
				layer->End();
		}

	} // namespace

	ImGuiLayer* Application::EnableImGui() {
		if (m_ImGuiLayer)
			return m_ImGuiLayer;

		m_ImGuiLayer = new ImGuiLayer(GetGraphicsBackend());
		m_BeginImGuiFrame = &BeginImGuiFrame;
		m_EndImGuiFrame = &EndImGuiFrame;
		PushOverlay(m_ImGuiLayer);
		return m_ImGuiLayer;
	}

} // namespace Ehu
