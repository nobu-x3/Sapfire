#include "core/asserts.h"
#include "logger.h"
#include "platform/platform.h"
// NOTE: temp
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

b8 logging_initialize() {
		// TODO: should open login file
		SF_INFO("Logging subsystem initialized sucessfully.");
		return TRUE;
}
void logging_shutdown() {
		// TODO: cleanup write queue
}

const char *level_string[6] = {"[FATAL]:	", "[ERROR]:	", "[WARNING]:	",
							   "[INFO]:	",	   "[DEBUG]:	", "[TRACE]:	"};

void log_output(log_level level, const char *message, ...) {
		// NOTE: I hope nobody goes over 32k bytes
		char formatted_message[32000];
		memset(formatted_message, 0, sizeof(formatted_message));

		char color_code[10];
		switch (level) {
		case LOG_LEVEL_DEBUG:
		case LOG_LEVEL_TRACE:
				strcpy(color_code, COLOR_BLUE);
				break;
		case LOG_LEVEL_INFO:
				strcpy(color_code, COLOR_GREEN);
				break;
		case LOG_LEVEL_WARNING:
				strcpy(color_code, COLOR_YELLOW);
				break;
		case LOG_LEVEL_ERROR:
		case LOG_LEVEL_FATAL:
				strcpy(color_code, COLOR_RED);
				break;
		}

		// NOTE: apparently MS headers override Clang's va_list with <typedef
		// char* va_list>, so this is a workaround
		__builtin_va_list arg_ptr;
		va_start(arg_ptr, message);
		vsnprintf(formatted_message, 32000, message, arg_ptr);
		va_end(arg_ptr);

		char out_message[32000];
		sprintf(out_message, "%s%s%s\033[0m\n", color_code, level_string[level],
				formatted_message);
		platform_console_write(out_message, level);
}

void report_assertion_failure(const char *expression, const char *message,
							  const char *file, i32 line) {
		log_output(
			LOG_LEVEL_FATAL,
			"Assertion Failure: %s, message: '%s', in file: %s, line %d\n",
			expression, message, file, line);
}
