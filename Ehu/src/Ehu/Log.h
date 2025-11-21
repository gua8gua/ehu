#pragma once
#include "ehupch.h"
#include "Core.h"
#include "spdlog/spdlog.h"

namespace Ehu {
	class EHU_API Log
	{
		public:
			static void Init();
			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define EHU_CORE_ERROR(...)		::Ehu::Log::GetCoreLogger()->error(__VA_ARGS__)
#define EHU_CORE_WARN(...)		::Ehu::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define EHU_CORE_INFO(...)		::Ehu::Log::GetCoreLogger()->info(__VA_ARGS__)
#define EHU_CORE_TRACE(...)		::Ehu::Log::GetCoreLogger()->trace(__VA_ARGS__)

#define EHU_ERROR(...)			::Ehu::Log::GetClientLogger()->error(__VA_ARGS__)
#define EHU_WARN(...)			::Ehu::Log::GetClientLogger()->warn(__VA_ARGS__)
#define EHU_INFO(...)			::Ehu::Log::GetClientLogger()->info(__VA_ARGS__)
#define EHU_TRACE(...)			::Ehu::Log::GetClientLogger()->trace(__VA_ARGS__)


