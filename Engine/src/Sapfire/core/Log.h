#pragma once
#include "Sapfire/core/Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Sapfire
{
	class Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return mEngineLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return mClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> mEngineLogger;
		static std::shared_ptr<spdlog::logger> mClientLogger;
	};

#define ENGINE_TRACE(...) Log::GetEngineLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...) Log::GetEngineLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...) Log::GetEngineLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...) Log::GetEngineLogger()->error(__VA_ARGS__)
#define ENGINE_FATAL(...) Log::GetEngineLogger()->critical(__VA_ARGS__)
#define LOG_TRACE(...) Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...) Log::GetClientLogger()->critical(__VA_ARGS__)
}