#include "ehupch.h"
#include "Application.h"
#include "Log.h"
#include "GLFW/glfw3.h"

namespace Ehu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		EHU_ASSERT(s_Instance == nullptr, "Application already exists");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		//WindowResizeEvent resizeEvent(1280, 720);
		//if (resizeEvent.IsInCategory(EventCategoryApplication)) {
		//	EHU_TRACE("Resize event received!");
		//}
		//if (resizeEvent.IsInCategory(EventCategoryInput)) {
		//	EHU_TRACE("Input event received!");
		//}

		while (m_Running) {
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			for (Layer *layer : m_LayerStack ) {
				layer->OnUpdate();
			}

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		EHU_CORE_TRACE("Event: {0}", e.ToString());

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.Handled) break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer *layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverLayer(Layer *layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

}