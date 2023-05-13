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

/**
* @brief Initializes logging. If memory is NULL, will populate mem_size.
*
*	@param mem_size Holds the required memory size of the internal state.
* @param memory NULL if requesting memory size, otherwise allocated block of memory.
* @return TRUE on success; otherwise FALSE.
*/
b8 logging_initialize(u64 *mem_size, void *memory);
void logging_shutdown(void* memory);

SAPI void log_output(log_level level, const char* message, ...);

#define COLOR_RED       "\033[0;31m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_YELLOW    "\033[0;93m"
#define COLOR_BLUE      "\033[0;94m"
#define COLOR_FATAL			"\033[0;41;30m"
#define COLOR_WHITE			"\033[0;97m"
#define COLOR_GREY			"\033[0;37m"

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
