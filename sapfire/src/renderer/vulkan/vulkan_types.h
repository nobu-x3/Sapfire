#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>

typedef struct vulkan_context{
	VkInstance instance;
	VkAllocationCallbacks* allocator;
} vulkan_context;

#define VK_ASSERT_SUCCESS(expr, message)											\
{																															\
	SF_ASSERT(expr == VK_SUCCESS, message);											\
}
