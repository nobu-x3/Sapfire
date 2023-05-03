#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define DEBUG
typedef struct vulkan_context{
	VkInstance instance;
	VkAllocationCallbacks* allocator;
	VkSurfaceKHR surface;
	#if defined(DEBUG)
	VkDebugUtilsMessengerEXT debug_messenger;
	#endif
} vulkan_context;

typedef struct vulkan_device{
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
} vulkan_device;

#define VK_ASSERT_SUCCESS(expr, message)											\
{																															\
	SF_ASSERT(expr == VK_SUCCESS, message);											\
}
