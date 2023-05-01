#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "game_definitions.h"
#include <stdlib.h>

extern b8 create_game(game* out_game);

int main(void) {
  memory_initialize();
	game instance;
	if(!create_game(&instance)){
		SF_FATAL("Failed to create game!");
		return -1;
	}

	if(!instance.render || !instance.initialize || !instance.update){
		SF_FATAL("All function pointers in game instance must be assigned!")
		return -1;
	}


	if(! application_create(&instance)){
		SF_FATAL("Failed to create the application!")
		return -1;
	}
	char* str = get_mem_usage_str();
  SF_DEBUG(str);
	free(str);
	application_run();
  return 0;
}
