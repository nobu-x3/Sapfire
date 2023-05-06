#pragma once

#include "vulkan_types.h"

void vulkan_framebuffer_create(vulkan_context *context,
                               vulkan_render_pass *render_pass, u32 width,
                               u32 height, u32 attachment_count,
                               VkImageView *attachments,
                               vulkan_framebuffer *out_framebuffer);
void vulkan_framebuffer_destroy(vulkan_context *context,
                                vulkan_framebuffer *framebuffer);
