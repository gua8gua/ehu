#include "ehupch.h"
#include "Application.h"
#include "Log.h"
#include "GLFW/glfw3.h"

namespace Ehu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_EventHandler = new EventHandler();
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
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		EHU_CORE_TRACE("Event: {0}", e.ToString());
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}