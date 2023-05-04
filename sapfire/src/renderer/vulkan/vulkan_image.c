#include "core/logger.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan_image.h"
#include <vulkan/vulkan_core.h>

void vulkan_image_create(vulkan_context *context,
						 vulkan_image_info *create_info,
						 vulkan_image *out_image) {
		VkImageCreateInfo image_create_info = {
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		// NOTE: This was supposed to be a helper function with defaults to
		// construct an image but at this point I might as well pass a
		// VkImageCreateInfo here.
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = create_info->width;
		image_create_info.extent.height = create_info->height;
		image_create_info.extent.depth = 1; // TODO: make this configurable
		image_create_info.mipLevels =
			4; // TODO: make this configurable to support mipmapping
		image_create_info.arrayLayers = 1; // TODO: make this configurable
		image_create_info.format = create_info->format;
		image_create_info.tiling = create_info->tiling;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = create_info->usage_flags;
		image_create_info.samples =
			VK_SAMPLE_COUNT_1_BIT; // TODO: make this configurable
		image_create_info.sharingMode =
			VK_SHARING_MODE_EXCLUSIVE; // TODO: make this configurable
		VK_ASSERT_SUCCESS(vkCreateImage(context->device.logical_device,
										&image_create_info, context->allocator,
										&out_image->image_handle),
						  "Failed to create an image.");
		VkMemoryRequirements mem_reqs;
		vkGetImageMemoryRequirements(context->device.logical_device,
									 out_image->image_handle, &mem_reqs);
		i32 mem_type = context->find_memory_index(mem_reqs.memoryTypeBits,
												  create_info->memory_flags);
		if (mem_type == -1) {
				SF_ERROR("Required memory type not found.");
		}

		VkMemoryAllocateInfo mem_alloc_info = {
			VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO};
		mem_alloc_info.allocationSize = mem_reqs.size;
		mem_alloc_info.memoryTypeIndex = mem_type;
		VK_ASSERT_SUCCESS(vkAllocateMemory(context->device.logical_device,
										   &mem_alloc_info, context->allocator,
										   &out_image->memory),
						  "Failed to allocate memory for an image.");

		// TODO: make this configurable for image pooling support
		VK_ASSERT_SUCCESS(vkBindImageMemory(context->device.logical_device,
											out_image->image_handle,
											out_image->memory, 0),
						  "Failed to bind image memory.");
		if (create_info->create_view) {
				out_image->view = SF_NULL;
				vulkan_image_view_create(context, create_info->format,
										 create_info->view_aspect_flags,
										 out_image);
		}
		out_image->width = create_info->width;
		out_image->height = create_info->height;
}

void vulkan_image_destroy(vulkan_context *context, vulkan_image *image) {
		if (image->view) {
				vkDestroyImageView(context->device.logical_device, image->view,
								   context->allocator);
				image->view = SF_NULL;
		}
		if (image->memory) {
				vkFreeMemory(context->device.logical_device, image->memory,
							 context->allocator);
				image->memory = SF_NULL;
		}
		if (image->image_handle) {
				vkDestroyImage(context->device.logical_device,
							   image->image_handle, context->allocator);
				image->image_handle = SF_NULL;
		}
}

void vulkan_image_view_create(vulkan_context *context, VkFormat format,
							  VkImageAspectFlags aspect_flags,
							  vulkan_image *out_image) {
		VkImageViewCreateInfo view_create_info = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		view_create_info.image = out_image->image_handle;
		view_create_info.viewType =
			VK_IMAGE_VIEW_TYPE_2D; // TODO: make this configurable
		view_create_info.format = format;
		view_create_info.subresourceRange.aspectMask = aspect_flags;

		// TODO: make all this configurable...
		view_create_info.subresourceRange.baseMipLevel = 0;
		view_create_info.subresourceRange.levelCount = 1;
		view_create_info.subresourceRange.baseArrayLayer = 0;
		view_create_info.subresourceRange.layerCount = 1;
		VK_ASSERT_SUCCESS(
			vkCreateImageView(context->device.logical_device, &view_create_info,
							  context->allocator, &out_image->view),
			"Failed to create image view");
}
