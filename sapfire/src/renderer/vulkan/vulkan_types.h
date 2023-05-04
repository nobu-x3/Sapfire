#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define DEBUG

typedef struct vulkan_image_info {
  VkImageType image_type;
  u32 width, height;
  VkFormat format;
  VkImageTiling tiling; // should basically always be VK_IMAGE_TILING_OPTIMAL
  VkImageUsageFlags usage_flags;
  VkMemoryPropertyFlags memory_flags;
  b32 create_view;
  VkImageAspectFlags view_aspect_flags;
} vulkan_image_info;

typedef struct vulkan_image {
  VkImage image_handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width, height;
} vulkan_image;

typedef struct vulkan_swapchain_support_info {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR *formats;
  u32 present_mode_count;
  VkPresentModeKHR *present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_swapchain {
  VkSurfaceFormatKHR surface_format;
  u8 max_frames_in_flight;
  VkSwapchainKHR swapchain_handle;
  u32 image_count;
  VkImage *images;
  VkImageView *image_views;
  vulkan_image depth_attachment;
} vulkan_swapchain;

typedef struct vulkan_device {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  vulkan_swapchain_support_info swapchain_support;

  i32 graphics_queue_index;
  i32 present_queue_index;
  i32 transfer_queue_index;

  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;

  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;

  VkFormat depth_format;
} vulkan_device;

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks *allocator;
  VkSurfaceKHR surface;
  vulkan_device device;
  vulkan_swapchain swapchain;
  u32 image_index;
  u32 current_frame;
  b8 recreating_swapchain;
  u32 framebuffer_width, framebuffer_height;

#if defined(DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
  i32(*find_memory_index)(u32 type_filter, u32 memory_flags);

} vulkan_context;

#define VK_ASSERT_SUCCESS(expr, message)                                       \
  { SF_ASSERT(expr == VK_SUCCESS, message); }
