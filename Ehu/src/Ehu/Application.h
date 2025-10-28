#pragma once
#include "Core.h"
#include "EventHandler.h"
#include "Window.h"

namespace Ehu
{
	class EHU_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
	private:
		EventHandler* m_EventHandler;
		std::unique_ptr<Window> m_Window;
		bool m_Running;
	};
	
	Application* CreateApplication();
}