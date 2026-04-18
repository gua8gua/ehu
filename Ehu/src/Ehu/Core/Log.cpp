#include "ehupch.h"
#include "Log.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <mutex>

namespace Ehu {

	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	namespace {

		std::mutex s_BufferMutex;
		std::vector<BufferedLogMessage> s_Buffer;
		struct BufferedSink;
		Ref<BufferedSink> s_BufferSink;

		class BufferedSink final : public spdlog::sinks::base_sink<std::mutex> {
		protected:
			void sink_it_(const spdlog::details::log_msg& msg) override {
				auto levelText = spdlog::level::to_string_view(msg.level);
				BufferedLogMessage entry;
				entry.Logger.assign(msg.logger_name.data(), msg.logger_name.size());
				entry.Level.assign(levelText.data(), levelText.size());
				entry.Message = fmt::to_string(msg.payload);

				std::lock_guard<std::mutex> lock(s_BufferMutex);
				s_Buffer.push_back(std::move(entry));
				if (s_Buffer.size() > 500)
					s_Buffer.erase(s_Buffer.begin(), s_Buffer.begin() + (s_Buffer.size() - 500));
			}

			void flush_() override {}
		};

	}

	void Log::Init() {
#ifdef EHU_PLATFORM_WINDOWS
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
#endif
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_BufferSink = CreateRef<BufferedSink>();
		s_CoreLogger = spdlog::stdout_color_mt("Ehu");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->sinks().push_back(s_BufferSink);
		s_ClientLogger = spdlog::stdout_color_mt("App");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->sinks().push_back(s_BufferSink);
	}

	const std::vector<BufferedLogMessage>& Log::GetBufferedMessages() {
		return s_Buffer;
	}

	void Log::ClearBufferedMessages() {
		std::lock_guard<std::mutex> lock(s_BufferMutex);
		s_Buffer.clear();
	}

} // namespace Ehu
