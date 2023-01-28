#pragma once
#include "Core.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

class Log
{
	public:
	static void Init();
	inline static Ref<spdlog::logger> &GetEngineLogger() { return mEngineLogger; }
	inline static Ref<spdlog::logger> &GetClientLogger() { return mClientLogger; }

	private:
	static Ref<spdlog::logger> mEngineLogger;
	static Ref<spdlog::logger> mClientLogger;
};

#define ENGINE_TRACE(...) Log::GetEngineLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...) Log::GetEngineLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...) Log::GetEngineLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...) Log::GetEngineLogger()->error(__VA_ARGS__)
#define ENGINE_FATAL(...) Log::GetEngineLogger()->fatal(__VA_ARGS__)

#define TRACE(...) Log::GetClientLogger()->trace(__VA_ARGS__)
#define INFO(...) Log::GetClientLogger()->info(__VA_ARGS__)
#define WARN(...) Log::GetClientLogger()->warn(__VA_ARGS__)
#define ERROR(...) Log::GetClientLogger()->error(__VA_ARGS__)
#define FATAL(...) Log::GetClientLogger()->fatal(__VA_ARGS__)
