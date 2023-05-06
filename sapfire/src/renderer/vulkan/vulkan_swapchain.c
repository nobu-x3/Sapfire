#include "core/logger.h"
#include "core/sfmemory.h"
#include "defines.h"
#include "renderer/vulkan/vulkan_device.h"
#include "renderer/vulkan/vulkan_image.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_swapchain.h"
#include <stdint.h>

void vulkan_swapchain_create(vulkan_context *context, u32 width, u32 height,
							 vulkan_swapchain *out_swapchain,
							 VkSwapchainKHR old_swapchain_handle) {
		SF_INFO("Creating swapchain...");
		VkExtent2D swapchain_extent = {width, height};
		// TODO: figure out if we want tripple buffering or make it configurable
		out_swapchain->max_frames_in_flight = 2;
		b8 found = FALSE;
		for (u32 i = 0; i < context->device.swapchain_support.format_count;
			 ++i) {
				VkSurfaceFormatKHR surface_format =
					context->device.swapchain_support.formats[i];
				if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
					surface_format.colorSpace ==
						VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						out_swapchain->surface_format = surface_format;
						found = TRUE;
						break;
				}
		}

		if (!found) {
				SF_WARNING("VK_FORMAT_B8G8R8A8_UNORM format not found, using "
						   "fallback.");
				out_swapchain->surface_format =
					context->device.swapchain_support
						.formats[0]; // potentially unsafe :^)
		}
		SF_INFO("Surface format found.");
		VkPresentModeKHR present_mode =
			VK_PRESENT_MODE_FIFO_KHR; // this is guaranteed to exist.
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPresentModeKHR.html
		for (u32 i = 0;
			 i < context->device.swapchain_support.present_mode_count; ++i) {
				VkPresentModeKHR mode =
					context->device.swapchain_support.present_modes[i];
				// By default enables tripple buffering. Use most current image,
				// discard the others.
				if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
						present_mode = mode;
						break;
				}
		}

		// handles display changes
		vulkan_device_query_swapchain_support(
			context->device.physical_device, context->surface,
			&context->device.swapchain_support);

		if (context->device.swapchain_support.capabilities.currentExtent
					.width != UINT32_MAX &&
			context->device.swapchain_support.capabilities.currentExtent
					.height != UINT32_MAX) {
				swapchain_extent = context->device.swapchain_support
									   .capabilities.currentExtent;
		}

		// can't have swapchain in bigger extent than the images
		VkExtent2D min_extent =
			context->device.swapchain_support.capabilities.minImageExtent;
		VkExtent2D max_extent =
			context->device.swapchain_support.capabilities.maxImageExtent;
		swapchain_extent.width =
			CLAMP(swapchain_extent.width, min_extent.width, max_extent.width);
		swapchain_extent.height = CLAMP(swapchain_extent.height,
										min_extent.height, max_extent.height);
		u32 image_count =
			context->device.swapchain_support.capabilities.minImageCount + 1;
		if (context->device.swapchain_support.capabilities.maxImageCount > 0 &&
			image_count >
				context->device.swapchain_support.capabilities.maxImageCount) {
				image_count = context->device.swapchain_support.capabilities
								  .maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchain_create_info = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.minImageCount = image_count;
		swapchain_create_info.imageFormat =
			out_swapchain->surface_format.format;
		swapchain_create_info.imageColorSpace =
			out_swapchain->surface_format.colorSpace;
		swapchain_create_info.imageExtent = swapchain_extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.surface = context->surface;
		if (context->device.graphics_queue_index ==
			context->device.present_queue_index) {
				swapchain_create_info.imageSharingMode =
					VK_SHARING_MODE_EXCLUSIVE;
				swapchain_create_info.queueFamilyIndexCount = 0;
				swapchain_create_info.pQueueFamilyIndices = SF_NULL;
		} else {
				u32 q_family_indices[] = {
					(u32)context->device.graphics_queue_index,
					(u32)context->device.present_queue_index};
				swapchain_create_info.imageSharingMode =
					VK_SHARING_MODE_CONCURRENT;
				swapchain_create_info.queueFamilyIndexCount = 2;
				swapchain_create_info.pQueueFamilyIndices = q_family_indices;
		}
		// portrait vs landscape
		swapchain_create_info.preTransform =
			context->device.swapchain_support.capabilities.currentTransform;
		// no transparency/compoisiting with OS
		swapchain_create_info.compositeAlpha =
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = present_mode;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = old_swapchain_handle;
		VK_ASSERT_SUCCESS(vkCreateSwapchainKHR(context->device.logical_device,
											   &swapchain_create_info,
											   context->allocator,
											   &out_swapchain->handle),
						  "Failed to create swapchain.");
		SF_INFO("Swapchain handle created.");
		context->current_frame = 0;
		out_swapchain->image_count = 0;
		VK_ASSERT_SUCCESS(
			vkGetSwapchainImagesKHR(context->device.logical_device,
									out_swapchain->handle,
									&out_swapchain->image_count, SF_NULL),
			"Failed to get swapchain image count.");
		if (!out_swapchain->images) {
				out_swapchain->images = (VkImage *)sfalloc(
					sizeof(VkImage) * out_swapchain->image_count,
					MEMORY_TAG_RENDERER);
		}
		if (!out_swapchain->image_views) {
				out_swapchain->image_views = (VkImageView *)sfalloc(
					sizeof(VkImageView) * out_swapchain->image_count,
					MEMORY_TAG_RENDERER);
		}
		VK_ASSERT_SUCCESS(
			vkGetSwapchainImagesKHR(
				context->device.logical_device, out_swapchain->handle,
				&out_swapchain->image_count, out_swapchain->images),
			"Failed to get swapchain images.");
		SF_INFO("Obtained swapchain images.");
		for (u32 i = 0; i < out_swapchain->image_count; ++i) {
				VkImageViewCreateInfo view_create_info = {
					VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
				view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				view_create_info.image = out_swapchain->images[i];
				view_create_info.format = out_swapchain->surface_format.format;
				view_create_info.subresourceRange.aspectMask =
					VK_IMAGE_ASPECT_COLOR_BIT;
				view_create_info.subresourceRange.baseMipLevel = 0;
				view_create_info.subresourceRange.levelCount = 1;
				view_create_info.subresourceRange.baseArrayLayer = 0;
				view_create_info.subresourceRange.layerCount = 1;
				VK_ASSERT_SUCCESS(
					vkCreateImageView(context->device.logical_device,
									  &view_create_info, context->allocator,
									  &out_swapchain->image_views[i]),
					"Failed to create image view.");
		}
		SF_INFO("Created image views for swapchain images.");
		if (!vulkan_device_detect_depth_format(&context->device)) {
				context->device.depth_format = VK_FORMAT_UNDEFINED;
				SF_FATAL("Failed to find a supported depth format.");
				return;
		}

		vulkan_image_info image_info = {VK_IMAGE_TYPE_2D};
		image_info.width = swapchain_extent.width;
		image_info.height = swapchain_extent.height;
		image_info.format = context->device.depth_format;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_info.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		image_info.create_view = TRUE;
		image_info.view_aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
		vulkan_image_create(context, &image_info,
							&out_swapchain->depth_attachment);
		SF_INFO("Swapchain created successfully.");
}

void vulkan_swapchain_destroy(vulkan_context *context,
							  vulkan_swapchain *swapchain) {
		vulkan_image_destroy(context, &swapchain->depth_attachment);
		// NOTE: this is due to the fact that when swapchain is created, images
		// are created with it and are automatically destroyed when the owning
		// swapchain is destroyed. Views that we created are not destroyed with
		// the owning swapchain.
		for (u32 i = 0; i < swapchain->image_count; ++i) {
				vkDestroyImageView(context->device.logical_device,
								   swapchain->image_views[i],
								   context->allocator);
		}
		vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle,
							  context->allocator);

		// NOTE: internal memory cleanup
		sffree((void *)swapchain->images,
			   sizeof(VkImage) * swapchain->image_count, MEMORY_TAG_RENDERER);
		sffree((void *)swapchain->image_views,
			   sizeof(VkImageView) * swapchain->image_count,
			   MEMORY_TAG_RENDERER);
}

void vulkan_swapchain_recreate(vulkan_context *context, u32 width, u32 height,
							   vulkan_swapchain *swapchain) {
		VkSwapchainKHR old_handle = swapchain->handle;
		vulkan_swapchain_destroy(context, swapchain);
		vulkan_swapchain_create(context, width, height, swapchain, old_handle);
}

b8 vulkan_swapchain_get_next_image_index(vulkan_context *context,
										 vulkan_swapchain *swapchain,
										 u64 timeout_ns,
										 VkSemaphore img_available_sem,
										 VkFence fence, u32 *out_image_index) {
		VkResult result = vkAcquireNextImageKHR(
			context->device.logical_device, swapchain->handle, timeout_ns,
			img_available_sem, fence, out_image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				vulkan_swapchain_recreate(context, context->framebuffer_width,
										  context->framebuffer_height,
										  swapchain);
				return FALSE;
		}

		// TODO: rework when implementing render passes
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				SF_FATAL("Failed to acquire the next image index.");
				return FALSE;
		}
		return TRUE;
}

void vulkan_swapchain_present(vulkan_context *context,
							  vulkan_swapchain *swapchain, VkQueue gfx_queue,
							  VkQueue present_queue,
							  VkSemaphore render_complete_sem,
							  u32 present_image_index) {
		// basically return the image to present to the swapchain
		VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		present_info.pSwapchains = &swapchain->handle;
		present_info.swapchainCount = 1;
		present_info.pImageIndices = &present_image_index;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_complete_sem;
		VkResult result = vkQueuePresentKHR(present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				vulkan_swapchain_recreate(context, context->framebuffer_width,
										  context->framebuffer_height,
										  swapchain);
		} else if (result != VK_SUCCESS) {
				SF_ERROR("Failed to present swapchain image.");
		}
}
