#pragma once

#include "renderer/renderer_types.h"
#include "vulkan_types.h"

b8 vulkan_shader_create (vulkan_context *context, struct texture* default_diffuse, vulkan_shader *out_shader);
void vulkan_shader_destroy (vulkan_context *context, vulkan_shader *shader);
void vulkan_shader_update_uniforms (vulkan_context *context,
									vulkan_shader *shader,
									scene_uniform *scene_data);
void vulkan_shader_update_model (vulkan_context *context, vulkan_shader *shader,
								 mesh_data data);
void vulkan_shader_bind (vulkan_context *context, vulkan_shader *shader);
b8 vulkan_shader_alloc(vulkan_context *context, vulkan_shader* shader, u32* out_id);
void vulkan_shader_free(vulkan_context* context, vulkan_shader* shader, u32 id);
