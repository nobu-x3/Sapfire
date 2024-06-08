#include "engpch.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "core/logger.h"

namespace Sapfire {

	stl::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	stl::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(stl::make_shared<spdlog::sinks::stdout_color_sink_mt>(mem::ENUM::Engine_Core));
		logSinks.emplace_back(stl::make_shared<spdlog::sinks::basic_file_sink_mt>(mem::ENUM::Engine_Core, "Sapfire.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_CoreLogger = stl::make_shared<spdlog::logger>(mem::ENUM::Engine_Core, "SAPFIRE", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = stl::make_shared<spdlog::logger>(mem::ENUM::Engine_Core, "APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}

	stl::shared_ptr<spdlog::logger>& Log::get_core_logger() { return s_CoreLogger; }

	stl::shared_ptr<spdlog::logger>& Log::get_client_logger() { return s_ClientLogger; }

} // namespace Sapfire
