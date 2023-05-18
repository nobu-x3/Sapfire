#include "sfmemory.h"
#include "sfstring.h"
#include <stdio.h>
#include <string.h> // TODO: get rid of it

u64 sfstrlen(const char *string) { return strlen(string); }

char *sfstrdup(const char *string) {
		u64 len = strlen(string);
		char *copy = sfalloc(len + 1, MEMORY_TAG_STRING);
		sfmemcpy(copy, string, len + 1);
		return copy;
}

b8 sfstreq(const char *a, const char *b) { return strcmp(a, b) == 0; }

i32 sfstrfmt(char *dest, const char *format, ...) {
		__builtin_va_list p_args;
		va_start(p_args, format);
		char buffer[16000];
		i32 written = vsnprintf(buffer, 16000, format, p_args);
		buffer[written] = 0;
		sfmemcpy(dest, format, written + 1);
		va_end(p_args);
		return written;
}