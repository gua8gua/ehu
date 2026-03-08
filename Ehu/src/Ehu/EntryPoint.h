#pragma once

#ifdef EHU_PLATFORM_WINDOWS
#include "Core/Log.h"
#include "Core/Application.h"
#include "Core/FileSystem.h"
#include <chrono>
#include <cstdlib>
#include <string>

extern Ehu::Application* Ehu::CreateApplication();

int main() {
	{
		auto _ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"EntryPoint.h:main_first\",\"message\":\"main() first line\",\"data\":{\"step\":0},\"timestamp\":" + std::to_string(_ts) + ",\"hypothesisId\":\"C\"}\n";
		Ehu::FileSystem::AppendTextFile("debug-8e1d5b.log", _line);
	}
	Ehu::Log::Init();
	EHU_CORE_INFO("Initialized Log!");

	auto app = Ehu::CreateApplication();
	{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"EntryPoint.h:post_create_app\",\"message\":\"CreateApplication done, before Run\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; Ehu::FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
	app->Run();
	delete app;
	return 0;
}
#endif
