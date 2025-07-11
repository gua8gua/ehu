#include "ehupch.h"
#include "Application.h"

namespace Ehu
{
	Application::Application()
	{
		m_EventHandler = new EventHandler();
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (true) {

		}
	}
}