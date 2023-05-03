#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define DEBUG

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device{
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
} vulkan_device;

typedef struct vulkan_context{
	VkInstance instance;
	VkAllocationCallbacks* allocator;
	VkSurfaceKHR surface;
	vulkan_device device;

	#if defined(DEBUG)
	VkDebugUtilsMessengerEXT debug_messenger;
	#endif

} vulkan_context;


#define VK_ASSERT_SUCCESS(expr, message)											\
{																															\
	SF_ASSERT(expr == VK_SUCCESS, message);											\
}
