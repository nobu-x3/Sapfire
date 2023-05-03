#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_events.h>
#include <SDL_log.h>
#include <SDL_mouse.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <string.h>

#include "containers/vector.h"
#include "core/event.h"
#include "core/input.h"
#include "core/logger.h"
#include "defines.h"
#include "platform.h"

typedef struct internal_state {
		SDL_Window *window;
		SDL_Surface *surface;
} internal_state;

b8 platform_init(platform_state *plat_state, const char *app_name, i32 x, i32 y,
				 i32 width, i32 height, u8 render_api) {
		plat_state->internal_state = malloc(sizeof(internal_state));
		internal_state *state = (internal_state *)plat_state->internal_state;
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
				SF_FATAL("Failed to initialize SDL!");
				return FALSE;
		}
		SDL_Vulkan_LoadLibrary(SF_NULL);
		state->window = SDL_CreateWindow(app_name, SDL_WINDOWPOS_CENTERED,
										 SDL_WINDOWPOS_CENTERED, width, height,
										 SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
		if (!state->window) {
				SF_FATAL("Failed to initialize window!");
				return FALSE;
		}
		// TODO: add a separate function to create a vulkan surface
		// state->surface = SDL_GetWindowSurface(state->window);
		// if (!state->surface) {
		// 		SF_FATAL("Failed to get surface!");
		// 		return FALSE;
		// }
		return TRUE;
}

void platform_shutdown(platform_state *plat_state) {
		internal_state *state = (internal_state *)plat_state->internal_state;
		if (state->window) {
				SDL_DestroyWindow(state->window);
		}
		SDL_Quit();
}

void platform_get_required_extension_names(struct platform_state *plat_state,
										   const char ***names_vec) {

		internal_state *state = (internal_state *)plat_state->internal_state;
		u32 ext_count = 0;
		const char **ext_names = SF_NULL;
		SDL_Vulkan_GetInstanceExtensions(state->window, &ext_count, SF_NULL);
		ext_names = platform_allocate(sizeof(const char *) * ext_count, FALSE);
		SDL_Vulkan_GetInstanceExtensions(state->window, &ext_count, ext_names);
		for (u32 i = 0; i < ext_count; ++i) {
				vector_push(*names_vec, ext_names[i]);
		}
		platform_free(ext_names, FALSE);
}

b8 platform_update_internal_state(platform_state *plat_state) {
		internal_state *state = (internal_state *)plat_state->internal_state;
		SDL_Event e;
		while (SDL_PollEvent(&e) > 0) {
				switch (e.type) {
				case SDL_QUIT:
						return FALSE;
				case SDL_KEYDOWN:
						input_process_key((keys)e.key.keysym.scancode, TRUE);
						break;
				case SDL_KEYUP:
						input_process_key((keys)e.key.keysym.scancode, FALSE);
						break;
				case SDL_MOUSEBUTTONDOWN:
						input_process_mouse_button(
							(mouse_button)e.button.button, TRUE);
						break;
				case SDL_MOUSEBUTTONUP:
						input_process_mouse_button(
							(mouse_button)e.button.button, FALSE);
						break;
				case SDL_MOUSEMOTION:
						input_process_mouse_move(e.motion.x, e.motion.y);
						break;
				case SDL_MOUSEWHEEL_NORMAL:
						input_process_mouse_wheel(e.wheel.y);
						break;
				}
		}
		SDL_UpdateWindowSurface(state->window);
		return TRUE;
}

void *platform_allocate(u64 size, b8 aligned) { return malloc(size); }

void platform_free(void *block, b8 aligned) { free(block); }

void *platform_set_memory(void *dest, i32 value, u64 size) {
		return memset(dest, value, size);
}

void *platform_copy_memory(void *dest, const void *source, u64 size) {
		return memcpy(dest, source, size);
}

void platform_console_write(const char *message, log_level level) {
		switch (level) {
		case LOG_LEVEL_DEBUG:
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", message);
				break;
		case LOG_LEVEL_INFO:
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", message);
				break;
		case LOG_LEVEL_TRACE:
				SDL_Log("%s", message);
				break;
		case LOG_LEVEL_WARNING:
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", message);
				break;
		case LOG_LEVEL_ERROR:
				platform_console_write_error(message, FALSE);
				break;
		case LOG_LEVEL_FATAL:
				platform_console_write_error(message, TRUE);
				break;
		}
}

void platform_console_write_error(const char *message, b8 fatal) {
		if (fatal) {
				SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", message);
		} else {
				SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", message);
		}
}

u64 platform_get_absolute_time() { return SDL_GetTicks64(); }

void platform_sleep(u32 ms) { SDL_Delay(ms); }
