#pragma once
#include "defines.h"
#include "logger.h"
#include "memory/lin_alloc.h"
#include "platform/platform.h"
#include "renderer/renderer_types.h"
#include "core/clock.h"
struct game;
typedef struct application_config{
	i32 x,y,width,height;
	const char* name;
} application_config;

typedef struct application_state{
	platform_state plat_state;

	// Application does not own game instance
	struct game* game_instance;
	renderer renderer;
	clock main_clock;
	f64 last_time;
	b8 is_running;
	linear_allocator systems_allocator;
	
	u64 logging_system_memory_size;
	void* logging_system;
	u64 event_system_memory_size;
	void* event_system;
	u64 input_system_memory_size;
	void* input_system;
} application_state;

/**
* @brief Creates application and event subsystems. Must be called before game_start (). This is the entry point for application - specific code.
* @param game_instance * Pointer to the game instance to be created.
* @return TRUE on success FALSE on failure ( memory exhaustion ). Note that applications are created in a single thread
*/
SAPI b8 application_create(struct game* game_instance);

/**
* @brief The function called by the game loop to start the application. This is where the game logic is executed
* @param game_instance * Pointer to the game
*/
SAPI void application_run(struct game* game_instance);

/**
* @brief Shut down the application. This is called at the end of each game to free memory allocated for the application state.
* @param game Game state to be shut down.
*/
SAPI void application_shutdown(struct game* game_instance);