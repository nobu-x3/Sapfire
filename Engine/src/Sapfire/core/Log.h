#pragma once
#include <spdlog/spdlog.h>

namespace Sapfire
{
	class Log
	{
	public:
		static void init();
		static std::shared_ptr<spdlog::logger>& get_engine_logger() { return mEngineLogger; }
		static std::shared_ptr<spdlog::logger>& get_client_logger() { return mClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> mEngineLogger;
		static std::shared_ptr<spdlog::logger> mClientLogger;
	};

#define ENGINE_TRACE(...) Log::get_engine_logger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...) Log::get_engine_logger()->info(__VA_ARGS__)
#define ENGINE_WARN(...) Log::get_engine_logger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...) Log::get_engine_logger()->error(__VA_ARGS__)
#define ENGINE_FATAL(...) Log::get_engine_logger()->critical(__VA_ARGS__)
#define LOG_TRACE(...) Log::get_client_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Log::get_client_logger()->info(__VA_ARGS__)
#define LOG_WARN(...) Log::get_client_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::get_client_logger()->error(__VA_ARGS__)
#define LOG_FATAL(...) Log::get_client_logger()->critical(__VA_ARGS__)
}