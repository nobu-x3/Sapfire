#pragma once

#include "core/stl/shared_ptr.h"

#pragma warning(push, 0)
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace spdlog {
	class logger;
}

namespace Sapfire {
	class SFAPI Log {
	public:
		static void Init();

		static stl::shared_ptr<spdlog::logger>& get_core_logger();
		static stl::shared_ptr<spdlog::logger>& get_client_logger();

	private:
		static stl::shared_ptr<spdlog::logger> s_CoreLogger;
		static stl::shared_ptr<spdlog::logger> s_ClientLogger;
	};
} // namespace Sapfire

// Core log macros
#define CORE_TRACE(...) ::Sapfire::Log::get_core_logger()->trace(__VA_ARGS__)
#define CORE_INFO(...) ::Sapfire::Log::get_core_logger()->info(__VA_ARGS__)
#define CORE_WARN(...) ::Sapfire::Log::get_core_logger()->warn(__VA_ARGS__)
#define CORE_ERROR(...) ::Sapfire::Log::get_core_logger()->error(__VA_ARGS__)
#define CORE_CRITICAL(...) ::Sapfire::Log::get_core_logger()->critical(__VA_ARGS__)

// Client log macros
#define CLIENT_TRACE(...) ::Sapfire::Log::get_client_logger()->trace(__VA_ARGS__)
#define CLIENT_INFO(...) ::Sapfire::Log::get_client_logger()->info(__VA_ARGS__)
#define CLIENT_WARN(...) ::Sapfire::Log::get_client_logger()->warn(__VA_ARGS__)
#define CLIENT_ERROR(...) ::Sapfire::Log::get_client_logger()->error(__VA_ARGS__)
#define CLIENT_CRITICAL(...) ::Sapfire::Log::get_client_logger()->critical(__VA_ARGS__)
