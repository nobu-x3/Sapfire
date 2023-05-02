#include "application.h"
#include "core/clock.h"
#include "core/event.h"
#include "core/input.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "game_definitions.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/renderer_types.h"

static b8 is_initialized = FALSE;
static application_state app_state;

b8 escape_pressed(u16 code, void *sender, void *listener_list,
				  event_context data);

b8 application_create(game *game_instance) {
		if (is_initialized) {
				SF_ERROR("applicated_create called more than once.");
				return FALSE;
		}

		// Init subsystems
		if (!event_initialize()) {
				SF_ERROR("Event system failed to initialize.");
				return FALSE;
		}
		logging_initialize();
		input_initialize();

		app_state.game_instance = game_instance;
		if (!platform_init(
				&app_state.plat_state, game_instance->app_config.name,
				game_instance->app_config.x, game_instance->app_config.y,
				game_instance->app_config.width,
				game_instance->app_config.height, 0)) {
				SF_FATAL("FAILED TO CREATE APP!");
				return FALSE;
		}
		if (!renderer_initialize(&app_state.renderer, RENDERER_API_VULKAN,
								 game_instance->app_config.name,
								 &app_state.plat_state)) {
				SF_FATAL("Failed to initialize renderer");
				return FALSE;
		}
		event_register(EVENT_CODE_KEY_PRESSED, &app_state, &escape_pressed);
		app_state.is_running = TRUE;
		is_initialized = TRUE;
		SF_INFO("Application initialized sucessfully.")
		return TRUE;
}

void application_run() {
		clock_start(&app_state.main_clock);
		clock_tick(&app_state.main_clock);
		app_state.last_time = (float)app_state.main_clock.elapsed_ticks / 1000;
		f64 running_time = 0;
		const f64 target_frame_time = 1.0f / 60;
		while (app_state.is_running) {
				clock_tick(&app_state.main_clock);
				f64 current_time =
					(float)app_state.main_clock.elapsed_ticks / 1000;
				f64 delta = current_time - app_state.last_time;
				f64 frame_start_time =
					(float)platform_get_absolute_time() / 1000;
				if (!platform_update_internal_state(&app_state.plat_state)) {
						app_state.is_running = FALSE;
				}

				// TODO: rework this awfulness
				render_bundle bundle;
				bundle.deltaTime = delta;
				renderer_draw_frame(&app_state.renderer, &bundle);

				f64 frame_end_time = (float)platform_get_absolute_time() / 1000;
				f64 frame_elapsed_time = frame_end_time - frame_start_time;
				running_time += frame_elapsed_time;
				f64 remaining_seconds = target_frame_time - frame_elapsed_time;
				if (remaining_seconds > 0) {
						u32 remaining_ms = remaining_seconds * 1000;
						platform_sleep(remaining_ms - 1);
				}
				app_state.last_time = current_time;
		}
		app_state.is_running = FALSE;

		// Cleanup
		application_shutdown();
		input_initialize();
		logging_shutdown();
		renderer_shutdown(&app_state.renderer);
		event_shutdown();
		memory_shutdown();
}

void application_shutdown() {
		if (is_initialized) {
				platform_shutdown(&app_state.plat_state);
				is_initialized = FALSE;
		}
}

b8 escape_pressed(u16 code, void *sender, void *listener_list,
				  event_context data) {
		if (data.data.u16[0] == KEY_ESCAPE) {
				SF_INFO("Quitting app...");
				app_state.is_running = FALSE;
				return TRUE;
		}
		return FALSE;
}
