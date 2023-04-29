#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED 	1
#define LOG_INFO_ENABLED 	1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if SAPRELEASE
#define LOG_DEBUG_ENABLED	0
#define LOG_TRACE_ENABLED	0
#endif

typedef enum log_level {
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_TRACE = 5
} log_level;

b8 initialize_logging();
void shutdown_logging();

SAPI void log_output(log_level level, const char* message, ...);

#define SF_FATAL(message, ...) log_output(LOG_LEVEL_FATAL,message, ##__VA_ARGS__);

#ifndef SF_ERROR
	#define SF_ERROR(message, ...) log_output(LOG_LEVEL_ERROR,message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
	#ifndef SF_WARNING
		#define SF_WARNING(message, ...) log_output(LOG_LEVEL_WARNING,message, ##__VA_ARGS__);
	#endif
#else
	#define SF_WARNING(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
	#ifndef SF_INFO
		#define SF_INFO(message, ...) log_output(LOG_LEVEL_INFO,message, ##__VA_ARGS__);
	#endif
#else
	#define SF_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
	#ifndef SF_DEBUG
		#define SF_DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG,message, ##__VA_ARGS__);
	#endif
#else
	#define SF_DEBUG(message, ...)
#endif


#if LOG_TRACE_ENABLED == 1
	#ifndef SF_TRACE
		#define SF_TRACE(message, ...) log_output(LOG_LEVEL_TRACE,message, ##__VA_ARGS__);
	#endif
#else
	#define SF_TRACE(message, ...)
#endif
