#include "application.h"
#include "core/logger.h"
#include "platform/platform.h"

application_state* application_create(application_config* config) {
  application_state* state =
      platform_allocate(sizeof(application_state), FALSE);
  logging_initialize();
  if (!platform_init(&state->plat_state, config->name, config->x, config->y,
                     config->width, config->height, 0)) {
    SF_FATAL("FAILED TO CREATE APP!");
    return FALSE;
  }
  state->is_running = TRUE;
  return state;
}

void application_run(application_state* state) {
  while (state->is_running) {
    if (!platform_update_internal_state(&state->plat_state)) {
      state->is_running = FALSE;
    }
  }
  state->is_running = FALSE;
  application_shutdown(state);
}

void application_shutdown(application_state* state) {
  if (state) {
    logging_shutdown();
    platform_shutdown(&state->plat_state);
    platform_free(state, FALSE);
  }
}