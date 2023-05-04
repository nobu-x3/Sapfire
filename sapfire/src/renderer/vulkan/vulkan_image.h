#pragma once

#include "vulkan_types.h"
#include <vulkan/vulkan_core.h>

void vulkan_image_create(vulkan_context* context, vulkan_image_info* create_info, vulkan_image* out_image);
void vulkan_image_destroy(vulkan_context* context, vulkan_image* image);
void vulkan_image_view_create(vulkan_context* context, VkFormat format, VkImageAspectFlags aspect_flags, vulkan_image* out_image);
