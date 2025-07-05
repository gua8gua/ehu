#pragma once
#ifdef EHU_PLATFORM_WINDOWS
#include "Ehu/Log.h"

extern Ehu::Application* Ehu::CreateApplication();

int main() {
	Ehu::Log::Init();
	EHU_CORE_INFO("Initialized Log!");

	auto app = Ehu::CreateApplication();
	app->Run();
	delete app;
	return 0;
}
#endif
