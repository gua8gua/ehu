#pragma once
#include "Core.h"
#include "Events/EventHandler.h"

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
	};
	
	Application* CreateApplication();
}