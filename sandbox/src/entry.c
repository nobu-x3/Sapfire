#include <entry.h>

#include "game.h"

b8 create_game(game* out_game) {
  out_game->app_config.name = "Sapfire Sandbox";
  out_game->app_config.width = 800;
  out_game->app_config.height = 600;
  out_game->initialize = game_init;
  out_game->update = game_update;
  out_game->render = game_render;
  return TRUE;
}
