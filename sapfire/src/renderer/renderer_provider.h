#pragma once

#include "renderer_types.h"

struct platform_state;

b8 renderer_provider_create(renderer_api api, struct platform_state* plat_state, renderer_provider* out_renderer_provider);
void renderer_provider_shutdown(renderer_provider* provider);
