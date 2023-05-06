#include "core/logger.h"
#include "vulkan_fence.h"
#include <vulkan/vulkan_core.h>

void vulkan_fence_create(vulkan_context *context, b8 signaled,
						 VkFence *out_fence) {
		VkFenceCreateInfo create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		if (signaled) {
				create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}
		VK_ASSERT_SUCCESS(vkCreateFence(context->device.logical_device,
										&create_info, context->allocator,
										out_fence),
						  "Failed to create fence.");
}

void vulkan_fence_destroy(vulkan_context *context, VkFence *fence) {
		if (fence) {
				vkDestroyFence(context->device.logical_device, *fence,
							   context->allocator);
				fence = SF_NULL;
		}
}
b8 vulkan_fence_wait(vulkan_context *context, VkFence *fence, u64 timeout_ns) {
		VkResult result = vkWaitForFences(context->device.logical_device, 1,
										  fence, TRUE, timeout_ns);
		switch (result) {
		case VK_SUCCESS:
				return TRUE;
		case VK_TIMEOUT:
				SF_ERROR("vulkan_fence_wait: timed out");
				break;
		case VK_ERROR_DEVICE_LOST:
				SF_ERROR("vulkan_fence_wait: VK_ERROR_DEVICE_LOST");
				break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				SF_ERROR("vulkan_fence_wait: VK_ERROR_OUT_OF_DEVICE_MEMORY");
				break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:

				SF_ERROR("vulkan_fence_wait: VK_ERROR_OUT_OF_HOST_MEMORY");
				break;
		default:
				SF_ERROR("vulkan_fence_wait: unknown error.");
				break;
		}
		return FALSE;
}

void vulkan_fence_reset(vulkan_context *context, VkFence *fence) {
		VK_ASSERT_SUCCESS(
			vkResetFences(context->device.logical_device, 1, fence),
			"Failed to wait for fence.");
}
