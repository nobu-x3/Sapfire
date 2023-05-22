#include "core/application.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "game_definitions.h"
#include <stdlib.h>

typedef b8(*PFN_create_game)(game* out_game);
typedef void(*PFN_game_shutdown)(game* game);

typedef struct callbacks{
	PFN_create_game create_game;
	PFN_game_shutdown game_shutdown;
} callbacks;

static callbacks instance;

b8 create_game(game* out_game){
	return instance.create_game(out_game);
}

void game_shutdown(game* game){
	instance.game_shutdown(game);
}

void set_create_game(PFN_create_game fn){
	instance.create_game = fn;
}

void set_game_shutdown(PFN_game_shutdown fn){
	instance.game_shutdown = fn;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) 
int start() {
  memory_initialize();
	game game_instance;
	if(!create_game(&game_instance)){
		SF_FATAL("Failed to create game!");
		return -1;
	}

	if(!game_instance.render || !game_instance.initialize || !game_instance.update){
		SF_FATAL("All function pointers in game instance must be assigned!")
		return -1;
	}


	if(!application_create(&game_instance)){
		SF_FATAL("Failed to create the application!")
		return -1;
	}
	char* str = get_mem_usage_str();
  	SF_DEBUG(str);
	free(str);
	application_run(&game_instance);
  return 0;
}
#else
int main() {
  memory_initialize();
	game game_instance;
	if(!create_game(&game_instance)){
		SF_FATAL("Failed to create game!");
		return -1;
	}

	if(!game_instance.render || !game_instance.initialize || !game_instance.update){
		SF_FATAL("All function pointers in game instance must be assigned!")
		return -1;
	}


	if(!application_create(&game_instance)){
		SF_FATAL("Failed to create the application!")
		return -1;
	}
	char* str = get_mem_usage_str();
  	SF_DEBUG(str);
	free(str);
	application_run(&game_instance);
  return 0;
}
#endif
