#include "core/asserts.h"
#include "core/sfmemory.h"
#include "core/sfstring.h"
#include "logger.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
// NOTE: temp
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct logger_state {
		file_handle file_handle;
} logger_state;

static logger_state *pState;

void write_to_log_file(const char *msg) {
		if (pState && pState->file_handle.is_valid) {
				u64 len = sfstrlen(msg);
				u64 written = 0;
				if (!filesystem_write(&pState->file_handle, len, msg,
									  &written)) {
						platform_console_write_error(
							"Unable to write to logs.log", TRUE);
				}
		}
}

b8 logging_initialize(u64 *mem_size, void *memory) {
		*mem_size = sizeof(logger_state);
		if (memory == SF_NULL) {
				return FALSE;
		}
		pState = memory;
		if (!filesystem_open("logs.log", FILE_MODE_WRITE, FALSE,
							 &pState->file_handle)) {
				platform_console_write_error(
					"Failed to open logs.log for writing", TRUE);
				return FALSE;
		}
		SF_INFO("Logging subsystem initialized sucessfully.");
		return TRUE;
}

void logging_shutdown(void *memory) {
		// TODO: cleanup write queue
		filesystem_close(&pState->file_handle);
		pState = SF_NULL;
}

const char *level_string[6] = {"[FATAL]:	", "[ERROR]:	", "[WARNING]:	",
							   "[INFO]:	",	   "[DEBUG]:	", "[TRACE]:	"};

void log_output(log_level level, const char *message, ...) {
		// NOTE: I hope nobody goes over 32k bytes
		char formatted_message[32000];
		sfmemset(formatted_message, 0, sizeof(formatted_message));

		char color_code[11];
		sfmemset(color_code, 0, sizeof(color_code));
		switch (level) {
		case LOG_LEVEL_DEBUG:
				strcpy(color_code, COLOR_WHITE);
				break;
		case LOG_LEVEL_TRACE:
				strcpy(color_code, COLOR_GREY);
				break;
		case LOG_LEVEL_INFO:
				strcpy(color_code, COLOR_GREEN);
				break;
		case LOG_LEVEL_WARNING:
				strcpy(color_code, COLOR_YELLOW);
				break;
		case LOG_LEVEL_ERROR:
				strcpy(color_code, COLOR_RED);
				break;
		case LOG_LEVEL_FATAL:
				strcpy(color_code, COLOR_FATAL);
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
		write_to_log_file(out_message);
}

void report_assertion_failure(const char *expression, const char *message,
							  const char *file, i32 line) {
		log_output(
			LOG_LEVEL_FATAL,
			"Assertion Failure: %s, message: '%s', in file: %s, line %d\n",
			expression, message, file, line);
}
