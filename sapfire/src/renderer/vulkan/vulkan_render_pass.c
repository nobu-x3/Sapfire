#include "core/logger.h"
#include "core/sfmemory.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan_render_pass.h"
#include <vulkan/vulkan_core.h>

void vulkan_render_pass_create(vulkan_context *context, color color,
							   extent2d extent, f32 depth, u32 stencil,
							   vulkan_render_pass *out_render_pass) {
		out_render_pass->depth = depth;
		out_render_pass->stencil = stencil;
		out_render_pass->color = color;
		out_render_pass->extent = extent;

		VkAttachmentDescription color_attachment;
		color_attachment.format =
			context->swapchain.surface_format.format; // TODO: config
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment_ref;
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment;
		depth_attachment.format = context->device.depth_format;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depth_attachment_ref;
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// TODO: other attachment types

		VkAttachmentDescription attachments[] = {color_attachment,
												 depth_attachment};

		VkSubpassDescription subpass;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		// TODO: other attachments
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = SF_NULL;
		subpass.pResolveAttachments = SF_NULL;
		subpass.preserveAttachmentCount = 0;
		subpass.pResolveAttachments = SF_NULL;

		// TODO: config
		VkSubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
								   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_create_info = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass;
		render_pass_create_info.attachmentCount = 2;
		render_pass_create_info.pAttachments = attachments;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &dependency;

		VK_ASSERT_SUCCESS(vkCreateRenderPass(context->device.logical_device,
											 &render_pass_create_info,
											 context->allocator,
											 &out_render_pass->handle),
						  "Failed to create subpass");
}

void vulkan_render_pass_destroy(vulkan_context *context,
								vulkan_render_pass *render_pass) {

		if (render_pass && render_pass->handle) {
				vkDestroyRenderPass(context->device.logical_device,
									render_pass->handle, context->allocator);
				render_pass->handle = SF_NULL;
		}
}

void vulkan_render_pass_begin(vulkan_render_pass *render_pass,
							  vulkan_command_buffer *command_buffer,
							  VkFramebuffer target_framebuffer) {
		VkClearValue clear_vals[2];
		sfmemset(clear_vals, 0, sizeof(VkClearValue) * 2);
		clear_vals[0].color.float32[0] = render_pass->color.r;
		clear_vals[0].color.float32[1] = render_pass->color.g;
		clear_vals[0].color.float32[2] = render_pass->color.b;
		clear_vals[0].color.float32[3] = render_pass->color.a;

		clear_vals[1].depthStencil.depth = render_pass->depth;
		clear_vals[1].depthStencil.stencil = render_pass->stencil;

		VkRenderPassBeginInfo begin_info = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		begin_info.renderPass = render_pass->handle;
		begin_info.renderArea.extent.width = render_pass->extent.w;
		begin_info.renderArea.extent.height = render_pass->extent.h;
		begin_info.renderArea.offset.x = render_pass->extent.x;
		begin_info.renderArea.offset.y = render_pass->extent.y;
		begin_info.framebuffer = target_framebuffer;
		begin_info.clearValueCount = 2;
		begin_info.pClearValues = clear_vals;

		// SF_DEBUG("HAI1");
		vkCmdBeginRenderPass(command_buffer->handle, &begin_info,
							 VK_SUBPASS_CONTENTS_INLINE);
		// SF_DEBUG("HAI2");
		command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_render_pass_end(vulkan_render_pass *render_pass,
							vulkan_command_buffer *command_buffer) {
		vkCmdEndRenderPass(command_buffer->handle);
		command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}
