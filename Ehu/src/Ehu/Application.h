#pragma once
#include "Core.h"
#include "Window.h"
#include "LayerStack.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"

namespace Ehu
{
	class EHU_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverLayer(Layer* layer);
	private:
		bool OnWindowClose(WindowCloseEvent &event);

		LayerStack m_LayerStack;
		std::unique_ptr<Window> m_Window;
		bool m_Running;
	};
	
	Application* CreateApplication();
}