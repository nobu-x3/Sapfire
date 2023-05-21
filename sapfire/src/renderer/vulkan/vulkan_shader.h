#pragma once

#include "renderer/renderer_types.h"
#include "vulkan_types.h"

b8 vulkan_shader_create(vulkan_context *context, vulkan_shader *out_shader);
void vulkan_shader_destroy(vulkan_context *context, vulkan_shader *shader);
void vulkan_shader_update_uniforms(vulkan_context *context,
                                   vulkan_shader *shader,
                                   scene_data *scene_data);
void vulkan_shader_update_model(vulkan_context* context, vulkan_shader* shader, mat4 model);
void vulkan_shader_bind(vulkan_context *context, vulkan_shader *shader);
