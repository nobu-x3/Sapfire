#include "core/logger.h"
#include "core/sfmemory.h"
#include "game.h"

b8 game_init(game* game_instance) {
  SF_DEBUG("Game initialized");
  return TRUE;
}
b8 game_update(game* game_instance, f32 deltaTime) { return TRUE; }
b8 game_render(game* game_instance, f32 deltaTime) { return TRUE; }
