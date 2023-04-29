#include "logger.h"
#include "core/asserts.h"
// NOTE: temp
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

b8 initialize_logging(){
	// TODO: should open login file
	return TRUE;
}
void shutdown_logging(){
	// TODO: cleanup write queue
}

const char* level_string[6] = {"[FATAL]:	", "[ERROR]:	", "[WARNING]:	", "[INFO]:	", "[DEBUG]:	", "[TRACE]:	"};

void log_output(log_level level, const char* message, ...){
	// NOTE: I hope nobody goes over 32k bytes
	char formatted_message[32000];	
	memset(formatted_message, 0, sizeof(formatted_message));

	// NOTE: apparently MS headers override Clang's va_list with <typedef char* va_list>, so this is a workaround
	__builtin_va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(formatted_message, 32000, message, arg_ptr);
	va_end(arg_ptr);

	char out_message[32000];	
	sprintf(out_message, "%s%s\n", level_string[level], formatted_message);

	// TODO: make platform specific
	printf("%s", out_message);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
	log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line %d\n", expression, message, file, line);	
}

