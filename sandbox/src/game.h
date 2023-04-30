#pragma once

#include <game_definitions.h>

typedef struct game_state{
	u32 placeholder;
} game_state;

b8 game_init(game* game_instance);
b8 game_update(game* game_instance, f32 deltaTime);
b8 game_render(game* game_instance, f32 deltaTime);

