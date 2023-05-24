#include "application.h"
#include "core/clock.h"
#include "core/event.h"
#include "core/input.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "entry.h"
#include "game_definitions.h"
#include "memory/lin_alloc.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "renderer/renderer_types.h"

b8 application_create (game *game_instance) {
	// Called more than once.
	if (game_instance->application_state) {
		SF_ERROR ("applicated_create called more than once.");
		return FALSE;
	}

	game_instance->application_state =
		sfalloc (sizeof (application_state), MEMORY_TAG_APPLICATION);
	application_state *app_state = game_instance->application_state;

	u64 systems_alloc_total_size = 64 * 1024 * 1024;
	linear_allocator_create (systems_alloc_total_size, SF_NULL,
							 &app_state->systems_allocator);

	// Init subsystems
	event_initialize (&app_state->event_system_memory_size, SF_NULL);
	app_state->event_system = linear_allocator_alloc (
		&app_state->systems_allocator, app_state->event_system_memory_size);
	// Initialize the event system.
	if (!event_initialize (&app_state->event_system_memory_size,
						   app_state->event_system)) {
		SF_FATAL ("Event system failed to initialize.");
		return FALSE;
	}

	logging_initialize (&app_state->logging_system_memory_size, SF_NULL);
	app_state->logging_system = linear_allocator_alloc (
		&app_state->systems_allocator, app_state->logging_system_memory_size);
	// Initialize logging system. If logging_initialize is called before the application is started.
	if (!logging_initialize (&app_state->logging_system_memory_size,
							 app_state->logging_system)) {
		SF_FATAL ("Failed to initialize logging.")
		return FALSE;
	}

	input_initialize (&app_state->input_system_memory_size, SF_NULL);
	app_state->input_system = linear_allocator_alloc (
		&app_state->systems_allocator, app_state->input_system_memory_size);
	// Initialize the input system.
	if (!input_initialize (&app_state->input_system_memory_size,
						   app_state->input_system)) {
		SF_FATAL ("Failed to initialize input system.");
		return FALSE;
	}

	// Creates a new app.
	if (!platform_init (&app_state->plat_state, game_instance->app_config.name,
						game_instance->app_config.x,
						game_instance->app_config.y,
						game_instance->app_config.width,
						game_instance->app_config.height, 0)) {
		SF_FATAL ("FAILED TO CREATE APP!");
		return FALSE;
	}
	// Initialize the renderer
	if (!renderer_initialize (&app_state->renderer, RENDERER_API_VULKAN,
							  game_instance->app_config.name,
							  &app_state->plat_state)) {
		SF_FATAL ("Failed to initialize renderer");
		return FALSE;
	}
	SF_INFO ("Application initialized sucessfully.")
	return TRUE;
}

void application_run (game *game_instance) {
	application_state *app_state = game_instance->application_state;
	app_state->is_running		 = TRUE;
	clock_start (&app_state->main_clock);
	clock_tick (&app_state->main_clock);
	app_state->last_time = (float)app_state->main_clock.elapsed_ticks / 1000;
	f64 running_time	 = 0;
	const f64 target_frame_time = 1.0f / 60;
	while (app_state->is_running) {
		clock_tick (&app_state->main_clock);
		f64 current_time = (float)app_state->main_clock.elapsed_ticks / 1000;
		f64 delta		 = current_time - app_state->last_time;
		f64 frame_start_time = (float)platform_get_absolute_time () / 1000;
		if (!platform_update_internal_state (&app_state->plat_state)) {
			app_state->is_running = FALSE;
		}

		// TODO: rework this awfulness
		render_bundle bundle;
		bundle.deltaTime = delta;
		renderer_draw_frame (&app_state->renderer, &bundle);

		f64 frame_end_time	   = (float)platform_get_absolute_time () / 1000;
		f64 frame_elapsed_time = frame_end_time - frame_start_time;
		running_time += frame_elapsed_time;
		f64 remaining_seconds = target_frame_time - frame_elapsed_time;
		if (remaining_seconds > 0) {
			u32 remaining_ms = remaining_seconds * 1000;
			platform_sleep (remaining_ms - 1);
		}
		app_state->last_time = current_time;
	}
	app_state->is_running = FALSE;

	// Cleanup
	game_shutdown (game_instance);
	application_shutdown (game_instance);
	input_shutdown (app_state->input_system);
	logging_shutdown (app_state->logging_system);
	renderer_shutdown (&app_state->renderer);
	event_shutdown (app_state->event_system);
	linear_allocator_destroy (&app_state->systems_allocator);
	memory_shutdown ();
}

void application_shutdown (game *game) {
	application_state *app_state = game->application_state;
	platform_shutdown (&app_state->plat_state);
	sffree (game->application_state, sizeof (application_state),
			MEMORY_TAG_APPLICATION);
}
