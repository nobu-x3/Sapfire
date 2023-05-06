#pragma once

#include "defines.h"
#include "vulkan_types.h"

void vulkan_render_pass_create(vulkan_context* context, color color, extent2d extent, f32 depth, u32 stencil, vulkan_render_pass* out_render_pass);
void vulkan_render_pass_destroy(vulkan_context* context, vulkan_render_pass* render_pass);
void vulkan_render_pass_begin(vulkan_render_pass* render_pass, vulkan_command_buffer* command_buffer, VkFramebuffer target_framebuffer);
void vulkan_render_pass_end(vulkan_render_pass* render_pass, vulkan_command_buffer* command_buffer);
