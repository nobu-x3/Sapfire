#include <entry.h>

#include "game.h"
#include <core/application.h>
#include <core/sfmemory.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

b8 _create_game (game *out_game) {
	out_game->app_config.name	= "Sapfire Sandbox";
	out_game->app_config.width	= 800;
	out_game->app_config.height = 600;
	out_game->initialize		= game_init;
	out_game->update			= game_update;
	out_game->render			= game_render;
	out_game->state	   = sfalloc (sizeof (game_state), MEMORY_TAG_GAME);
	game_state *state  = (game_state *)out_game->state;
	state->placeholder = 19;
	return TRUE;
}

void _game_shutdown (game *game) {
	sffree (game->state, sizeof (game_state), MEMORY_TAG_GAME);
}

int main () {
	set_create_game (_create_game);
	set_game_shutdown (_game_shutdown);
	start ();
}
#else

b8 create_game (game *out_game) {
	out_game->app_config.name	= "Sapfire Sandbox";
	out_game->app_config.width	= 800;
	out_game->app_config.height = 600;
	out_game->initialize		= game_init;
	out_game->update			= game_update;
	out_game->render			= game_render;
	out_game->state	   = sfalloc (sizeof (game_state), MEMORY_TAG_GAME);
	game_state *state  = (game_state *)out_game->state;
	state->placeholder = 19;
	return TRUE;
}

void game_shutdown (game *game) {
	sffree (game->state, sizeof (game_state), MEMORY_TAG_GAME);
}

#endif