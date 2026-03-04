#include "ehupch.h"
#include "ImGuiLayer.h"
#include "Core/Application.h"
#include "imgui.h"

namespace Ehu {

	ImGuiLayer::ImGuiLayer(GraphicsBackend backend)
		: Layer("ImGuiLayer")
		, m_Backend(ImGuiBackend::Create(backend))
	{
	}

	ImGuiLayer::~ImGuiLayer() {
		if (m_Backend)
			m_Backend->Shutdown();
	}

	void ImGuiLayer::OnAttach() {
		if (m_Backend)
			m_Backend->Init(&Application::Get().GetWindow());
	}

	void ImGuiLayer::OnDetach() {
	}

	void ImGuiLayer::Begin() {
		if (m_Backend)
			m_Backend->BeginFrame();
	}

	void ImGuiLayer::End() {
		if (m_Backend)
			m_Backend->EndFrame(&Application::Get().GetWindow());
	}

	void ImGuiLayer::OnImGuiRender() {
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::SetDarkThemeColors() {
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const {
		return 33;
	}

} // namespace Ehu
