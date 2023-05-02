#pragma once

#include "renderer/renderer_provider.h"

b8 vulkan_initialize(renderer_provider* api, const char* app_name, struct platform_state* plat_state);
void vulkan_shutdown(renderer_provider* api);
b8 vulkan_begin_frame(struct renderer_provider* api, f64 deltaTime);
b8 vulkan_end_frame(struct renderer_provider* api);