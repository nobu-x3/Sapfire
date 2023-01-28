#include "Log.h"
#include "engpch.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::mEngineLogger;
std::shared_ptr<spdlog::logger> Log::mClientLogger;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	mEngineLogger = spdlog::stdout_color_mt("ENGINE");
	mEngineLogger->set_level(spdlog::level::trace);

	mClientLogger = spdlog::stdout_color_mt("CLIENT");
	mClientLogger->set_level(spdlog::level::trace);
}
