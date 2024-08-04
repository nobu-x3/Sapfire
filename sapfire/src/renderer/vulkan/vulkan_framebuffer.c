#include "vulkan_framebuffer.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "renderer/vulkan/vulkan_types.h"
#include <vulkan/vulkan_core.h>

void vulkan_framebuffer_create (vulkan_context *context,
								vulkan_render_pass *render_pass, u32 width,
								u32 height, u32 attachment_count,
								VkImageView *attachments,
								vulkan_framebuffer *out_framebuffer) {
	out_framebuffer->attachments =
		sfalloc (sizeof (VkImageView) * attachment_count, MEMORY_TAG_RENDERER);
	for (u32 i = 0; i < attachment_count; ++i) {
		out_framebuffer->attachments[i] = attachments[i];
	}
	out_framebuffer->render_pass	  = render_pass;
	out_framebuffer->attachment_count = attachment_count;

	VkFramebufferCreateInfo create_info = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	create_info.attachmentCount = attachment_count;
	create_info.pAttachments	= out_framebuffer->attachments;
	create_info.renderPass		= render_pass->handle;
	create_info.width			= width;
	create_info.height			= height;
	create_info.layers			= 1;

	VK_ASSERT_SUCCESS (vkCreateFramebuffer (context->device.logical_device,
											&create_info, context->allocator,
											&out_framebuffer->handle),
					   "Failed to create framebuffer.");
	SF_INFO ("Framebuffer created.");
}

void vulkan_framebuffer_destroy (vulkan_context *context,
								 vulkan_framebuffer *framebuffer) {
	vkDestroyFramebuffer (context->device.logical_device, framebuffer->handle,
						  context->allocator);
	framebuffer->handle = SF_NULL;
	sffree (framebuffer->attachments,
			sizeof (VkImageView) * framebuffer->attachment_count,
			MEMORY_TAG_RENDERER);
	framebuffer->attachments	  = SF_NULL;
	framebuffer->attachment_count = 0;
	framebuffer->render_pass	  = SF_NULL;
}
