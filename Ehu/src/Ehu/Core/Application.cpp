#include "ehupch.h"
#include "Application.h"
#include "Log.h"
#include "Platform/GraphicsBackend.h"
#include "Platform/Input.h"
#include "Platform/RenderContext.h"
#include "Platform/RendererAPI.h"
#include "Renderer/Renderer.h"

namespace Ehu {

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		EHU_ASSERT(s_Instance == nullptr, "Application already exists");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(EHU_BIND_EVENT_FN(Application::OnEvent));

		Input::Init();
		RenderContext::Init();
		RenderContext::SetCurrentWindow(m_Window.get());
		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer(GetGraphicsBackend());
		PushLayer(m_ImGuiLayer);
	}

	Application::~Application() {
		Renderer::Shutdown();
		RenderContext::Shutdown();
	}

	void Application::Run() {
		while (m_Running) {
			RenderContext::GetAPI().SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			RenderContext::GetAPI().Clear(RendererAPI::ClearColor | RendererAPI::ClearDepth);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(EHU_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.Handled) break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverLayer(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

} // namespace Ehu
