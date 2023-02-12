#include "engpch.h"
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Sapfire
{
	std::shared_ptr<spdlog::logger> Log::mEngineLogger;
	std::shared_ptr<spdlog::logger> Log::mClientLogger;

	void Log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		mEngineLogger = spdlog::stdout_color_mt("ENGINE");
		mEngineLogger->set_level(spdlog::level::trace);

		mClientLogger = spdlog::stdout_color_mt("CLIENT");
		mClientLogger->set_level(spdlog::level::trace);
	}
}