#pragma once
#ifdef EHU_PLATFORM_WINDOWS

#include <stdio.h>
extern Ehu::Application* Ehu::CreateApplication();

void main() {
	printf("Hello, World!\n");
	auto app = Ehu::CreateApplication();
	app->Run();
	delete app;
}
#endif