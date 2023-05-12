#pragma once
#include "defines.h"
#include "logger.h"
#include "platform/platform.h"
#include "renderer/renderer_types.h"
#include "core/clock.h"
struct game;
typedef struct application_config{
	i32 x,y,width,height;
	const char* name;
} application_config;

typedef struct application_state{
	platform_state plat_state;
	struct game* game_instance;
	renderer renderer;
	clock main_clock;
	f64 last_time;
	b8 is_running;
} application_state;

SAPI b8 application_create(struct game* game_instance);
SAPI void application_run(struct game* game_instance);
SAPI void application_shutdown(struct game* game_instance);