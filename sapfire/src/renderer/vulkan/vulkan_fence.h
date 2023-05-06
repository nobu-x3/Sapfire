#pragma once

#include "vulkan_types.h"

void vulkan_fence_create(vulkan_context* context, b8 signaled, VkFence* out_fence);
void vulkan_fence_destroy(vulkan_context* context, VkFence* fence);
b8 vulkan_fence_wait(vulkan_context* context, VkFence* fence, u64 timeout_ns);
void vulkan_fence_reset(vulkan_context* context, VkFence* fence);
