#pragma once

#ifdef EHU_PLATFORM_WINDOWS
#include "Core/Log.h"
#include "Core/Application.h"

extern Ehu::Application* Ehu::CreateApplication(Ehu::ApplicationCommandLineArgs args);

int main(int argc, char** argv) {
	Ehu::Log::Init();
	Ehu::ApplicationCommandLineArgs args;
	args.Count = argc;
	args.Args = argv;
	auto app = Ehu::CreateApplication(args);
	app->Run();
	delete app;
	return 0;
}
#endif
