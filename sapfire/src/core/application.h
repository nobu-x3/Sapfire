#pragma once
#include "defines.h"
#include "logger.h"
#include "platform/platform.h"

struct game;

typedef struct application_config{
	i32 x,y,width,height;
	const char* name;
} application_config;

typedef struct application_state{
	platform_state plat_state;
	b8 is_running;
} application_state;

SAPI application_state* application_create(struct game* game_instance);
SAPI void application_run(application_state* state);
SAPI void application_shutdown(application_state* state);