#include "sfmemory.h"
#include "sfstring.h"
#include <string.h> // TODO: get rid of it

u64 sfstrlen(const char *string) { return strlen(string); }

char *sfstrdup(const char *string) {
		u64 len = strlen(string);
		char *copy = sfalloc(len + 1, MEMORY_TAG_STRING);
		sfmemcpy(copy, string, len + 1);
		return copy;
}

b8 sfstreq(const char *a, const char *b) { return strcmp(a, b) == 0; }
