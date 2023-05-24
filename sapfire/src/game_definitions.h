#pragma once

#include "core/application.h"

typedef struct game {
	application_config app_config;

	b8 (*initialize) (struct game* instance);
	b8 (*update) (struct game* instance, f32 deltaTime);
	b8 (*render) (struct game* instance, f32 deltaTime);

	void* state;

	void* application_state;
} game;
