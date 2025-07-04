#pragma once
#include "core.h"

namespace Ehu
{
	class EHU_API Application
		{
		public:
			Application();
			virtual ~Application();
			void Run();
		};
	
	Application* CreateApplication();
}