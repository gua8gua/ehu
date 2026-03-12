#include "ehupch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ehu {

	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
#ifdef EHU_PLATFORM_WINDOWS
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
#endif
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Ehu");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger = spdlog::stdout_color_mt("App");
		s_ClientLogger->set_level(spdlog::level::trace);
	}

} // namespace Ehu
