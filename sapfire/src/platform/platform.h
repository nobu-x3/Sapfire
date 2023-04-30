#pragma once

#include "core/logger.h"
#include "defines.h"

typedef struct platform_state{
	void* internal_state;
} platform_state;

SAPI b8 platform_init(platform_state* plat_state, const char* app_name, i32 x, i32 y, i32 width, i32 height, u8 render_api);
SAPI void platform_shutdown(platform_state* plat_state);
SAPI void platform_update_internal_state(platform_state* plat_state);
void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_set_memory(void* dest, i32 value, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void platform_console_write(const char* message, log_level level);
void platform_console_write_error(const char* message, b8 fatal);
u64 platform_get_absolute_time();
void platform_sleep(u32 ms);