#include "application.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "game_definitions.h"
#include "platform/platform.h"

static b8 is_initialized = FALSE;
static application_state app_state;

b8 application_create(game* game_instance) {
  if (is_initialized) {
    SF_ERROR("applicated_create called more than once.");
    return FALSE;
  }
  app_state.game_instance = game_instance;
  if (!platform_init(&app_state.plat_state, game_instance->app_config.name,
                     game_instance->app_config.x, game_instance->app_config.y,
                     game_instance->app_config.width,
                     game_instance->app_config.height, 0)) {
    SF_FATAL("FAILED TO CREATE APP!");
    return FALSE;
  }
  app_state.is_running = TRUE;
  is_initialized = TRUE;
  return TRUE;
}

void application_run() {
  while (app_state.is_running) {
    if (!platform_update_internal_state(&app_state.plat_state)) {
      app_state.is_running = FALSE;
    }
  }
  app_state.is_running = FALSE;

  // Cleanup
  application_shutdown();
  logging_shutdown();
  memory_shutdown();
}

void application_shutdown() {
  if (is_initialized) {
    platform_shutdown(&app_state.plat_state);
    platform_free(&app_state, FALSE);
    is_initialized = FALSE;
  }
}