#include "ehupch.h"
#include "Application.h"
#include "Log.h"

namespace Ehu
{
	Application::Application()
	{
		m_EventHandler = new EventHandler();
		m_Window = std::unique_ptr<Window>(Window::Create());
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
			m_Window->OnUpdate();
		}
	}
}