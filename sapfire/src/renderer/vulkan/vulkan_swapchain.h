#pragma once

#include "vulkan_types.h"

void vulkan_swapchain_create(vulkan_context *context, u32 width, u32 height,
                             vulkan_swapchain *out_swapchain, VkSwapchainKHR old_swapchain_handle);

void vulkan_swapchain_destroy(vulkan_context *context,
                              vulkan_swapchain *swapchain);

void vulkan_swapchain_recreate(vulkan_context *context, u32 width, u32 height,
                               vulkan_swapchain *swapchain);

b8 vulkan_swapchain_get_next_image_index(vulkan_context *context,
                                         vulkan_swapchain *swapchain,
                                         u64 timeout_ns,
                                         VkSemaphore img_available_sem,
                                         VkFence fence, u32 *out_image_index);

void vulkan_swapchain_present(vulkan_context *context,
                              vulkan_swapchain *swapchain, VkQueue gfx_queue,
                              VkQueue present_queue,
                              VkSemaphore render_complete_sem,
                              u32 present_image_index);
