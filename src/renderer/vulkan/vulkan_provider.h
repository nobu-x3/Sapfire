#pragma once

#include "renderer/renderer_provider.h"
#include "renderer/renderer_types.h"
#include "resources/resource_types.h"

b8 vulkan_initialize (renderer_provider *api, const char *app_name,
					  struct platform_state *plat_state);
void vulkan_shutdown (renderer_provider *api);
b8 vulkan_begin_frame (struct renderer_provider *api, f64 deltaTime);
void vulkan_update_scene_data (mat4 projection, mat4 view);
void vulkan_update_objects_data (mesh_data data);
b8 vulkan_end_frame (struct renderer_provider *api);
void vulkan_create_texture(const char* name, u32 width, u32 height, u32 channels, b8 opaque, const u8* pixels, texture* out_texture);
void vulkan_destroy_texture(texture* texture);
