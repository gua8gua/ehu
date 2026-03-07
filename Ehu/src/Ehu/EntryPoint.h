#pragma once

#ifdef EHU_PLATFORM_WINDOWS
#include "Core/Log.h"
#include "Core/Application.h"
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <string>

extern Ehu::Application* Ehu::CreateApplication();

int main() {
	// #region agent log
	{
		auto _ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"EntryPoint.h:main_first\",\"message\":\"main() first line\",\"data\":{\"step\":0},\"timestamp\":" + std::to_string(_ts) + ",\"hypothesisId\":\"C\"}\n";
		for (const char* _p : { "debug-8e1d5b.log" }) { std::ofstream _f(_p, std::ios::app); if (_f) { _f << _line; _f.flush(); } }
		{ const char* t = std::getenv("TEMP"); if (t) { std::string _tp = std::string(t) + "\\debug-8e1d5b.log"; std::ofstream _f(_tp, std::ios::app); if (_f) { _f << _line; _f.flush(); } } }
	}
	// #endregion
	Ehu::Log::Init();
	EHU_CORE_INFO("Initialized Log!");

	auto app = Ehu::CreateApplication();
	// #region agent log
	{ std::ofstream _f("debug-8e1d5b.log", std::ios::app); if (_f) { _f << "{\"sessionId\":\"8e1d5b\",\"location\":\"EntryPoint.h:post_create_app\",\"message\":\"CreateApplication done, before Run\",\"data\":{\"ok\":1},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << ",\"hypothesisId\":\"C\"}\n"; _f.flush(); } }
	// #endregion
	app->Run();
	delete app;
	return 0;
}
#endif
