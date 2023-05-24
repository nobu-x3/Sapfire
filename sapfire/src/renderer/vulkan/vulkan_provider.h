#pragma once

#include "renderer/renderer_provider.h"

b8 vulkan_initialize (renderer_provider *api, const char *app_name,
					  struct platform_state *plat_state);
void vulkan_shutdown (renderer_provider *api);
b8 vulkan_begin_frame (struct renderer_provider *api, f64 deltaTime);
void vulkan_update_scene_data (mat4 projection, mat4 view);
void vulkan_update_objects_data (mat4 model);
b8 vulkan_end_frame (struct renderer_provider *api);