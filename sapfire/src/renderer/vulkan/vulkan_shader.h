#pragma once

#include "vulkan_types.h"
#include "renderer/renderer_types.h"

b8 vulkan_shader_create(vulkan_context* context, vulkan_shader* out_shader);
void vulkan_shader_destroy(vulkan_context* context, vulkan_shader* shader);
void vulkan_shader_bind(vulkan_context* context, vulkan_shader* shader);
