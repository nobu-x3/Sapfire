#include "expect.h"
#include "test_manager.h"
#include <core/logger.h>
#include <core/sfmemory.h>
#include <game_definitions.h>

typedef struct game_state {
		u32 placeholder;
} game_state;

b8 game_init(game *game_instance) {
		SF_DEBUG("Game initialized");
		return TRUE;
}
b8 game_update(game *game_instance, f32 deltaTime) { return TRUE; }
b8 game_render(game *game_instance, f32 deltaTime) { return TRUE; }

b8 create_game(game *out_game) {
		out_game->app_config.name = "Sapfire Sandbox";
		out_game->app_config.width = 800;
		out_game->app_config.height = 600;
		out_game->initialize = game_init;
		out_game->update = game_update;
		out_game->render = game_render;
		out_game->state = sfalloc(sizeof(game_state), MEMORY_TAG_GAME);
		game_state *state = (game_state *)out_game->state;
		state->placeholder = 19;
		return TRUE;
}

void game_shutdown(game *game) {
		sffree(game->state, sizeof(game_state), MEMORY_TAG_GAME);
}

u8 test() { return FALSE; }

int main(void) {
		test_manager_init();
		test_manager_register_test(test, "test test");
		test_manager_run_tests();
		return 0;
}
