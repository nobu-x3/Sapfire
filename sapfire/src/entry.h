#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_definitions.h"

extern b8 create_game(game* out_game);

int main(void) {
	game instance;
	if(!create_game(&instance)){
		SF_FATAL("Failed to create game!");
		return -1;
	}

	if(!instance.render || !instance.initialize || !instance.update){
		SF_FATAL("All function pointers in game instance must be assigned!")
		return -1;
	}

	application_state* app_state = application_create(&instance);
	if(!app_state){
		SF_FATAL("Failed to create the application!")
		return -1;
	}

	application_run(app_state);
  return 0;
}
