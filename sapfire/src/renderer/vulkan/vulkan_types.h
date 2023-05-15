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
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width, height;
} vulkan_image;

typedef enum vulkan_render_pass_state {
  RENDER_PASS_STATE_IDLE,
  RENDER_PASS_STATE_RECORDING,
  RENDER_PASS_STATE_IN_RENDER_PASS,
  RENDER_PASS_STATE_RECORDING_FINISHED,
  RENDER_PASS_STATE_SUBMITTED,
  RENDER_PASS_STATE_NOT_ALLOCATED
} vulkan_render_pass_state;

typedef struct vulkan_render_pass {
  VkRenderPass handle;
  f32 depth;
  color color;
  extent2d extent;
  u32 stencil;
  vulkan_render_pass_state state;
} vulkan_render_pass;

typedef struct vulkan_framebuffer {
  VkFramebuffer handle;
  u32 attachment_count;
  VkImageView *attachments;
  vulkan_render_pass *render_pass;
} vulkan_framebuffer;

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
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage *images;
  VkImageView *image_views;
  vulkan_image depth_attachment;
  vulkan_framebuffer *framebuffers;
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
  COMMAND_BUFFER_STATE_IDLE,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_FINISHED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
  VkCommandBuffer handle;
  vulkan_command_buffer_state state;
} vulkan_command_buffer;

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

  VkCommandPool graphics_command_pool;

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
  vulkan_render_pass main_render_pass;

  // vec
  vulkan_command_buffer *graphics_command_buffers;

  // vec
  VkSemaphore *image_available_semaphores;

  // vec
  VkSemaphore *queue_complete_semaphores;

  u32 in_flight_fences_count;
  VkFence *in_flight_fences;
  VkFence **images_in_flight;

  u32 image_index;
  u32 current_frame;
  b8 recreating_swapchain;
  u32 framebuffer_width, framebuffer_height;

#if defined(DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
  i32 (*find_memory_index)(u32 type_filter, u32 memory_flags);

} vulkan_context;

#define VK_ASSERT_SUCCESS(expr, message)                                       \
  { SF_ASSERT(expr == VK_SUCCESS, message); }
