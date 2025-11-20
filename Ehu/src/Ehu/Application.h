#pragma once
#include "Core.h"
#include "EventHandler.h"
#include "Window.h"

#include "KeyEvent.h"
#include "MouseEvent.h"
#include "ApplicationEvent.h"

namespace Ehu
{
	class EHU_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent &event);

		EventHandler* m_EventHandler;
		std::unique_ptr<Window> m_Window;
		bool m_Running;
	};
	
	Application* CreateApplication();
}