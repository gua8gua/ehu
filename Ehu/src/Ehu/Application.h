#pragma once
#include "Core.h"
#include "Window.h"
#include "LayerStack.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "Ehu/ImGui/ImGuiLayer.h"

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

		static  Application& Get(){return *s_Instance;}
		inline Window& GetWindow(){return *m_Window;}

	private:
		bool OnWindowClose(WindowCloseEvent &event);

		LayerStack m_LayerStack;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
	private:
		static Application* s_Instance;
	};
	
	Application* CreateApplication();
}