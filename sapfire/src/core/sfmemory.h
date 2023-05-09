#pragma once

#include "defines.h"

typedef enum memory_tag{
	MEMORY_TAG_UNKNOWN,
	MEMORY_TAG_LIN_ALLOC,
	MEMORY_TAG_GAME,
	MEMORY_TAG_VECTOR,
	MEMORY_TAG_RENDERER,
	MEMORY_TAG_STRING,

	MEMORY_TAG_MAX
} memory_tag;

void memory_initialize();
void memory_shutdown();

SAPI void* sfalloc(u64 size, memory_tag tag);
SAPI void sffree(void* block, u64 size, memory_tag tag);
SAPI void* sfmemcpy(void* dest, const void* src, u64 size);
SAPI void* sfmemset(void* dest, i32 val, u64 size);
SAPI char* get_mem_usage_str();