#pragma once

#include "renderer_types.h"

b8 renderer_initialize(renderer *renderer, renderer_api api,
					   const char *application_name,
					   struct platform_state *plat_state);
void renderer_shutdown(renderer *renderer);
b8 renderer_draw_frame(renderer* renderer, render_bundle* bundle);