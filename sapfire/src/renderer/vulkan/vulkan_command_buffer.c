#include "core/sfmemory.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan_command_buffer.h"
#include <vulkan/vulkan_core.h>

void vulkan_command_buffer_create(vulkan_context *context, VkCommandPool pool,
								  b8 is_primary,
								  vulkan_command_buffer *out_cmd_bfr) {
		sfmemset(out_cmd_bfr, 0, sizeof(vulkan_command_buffer));
		VkCommandBufferAllocateInfo alloc_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		alloc_info.commandPool = pool;
		alloc_info.commandBufferCount = 1;
		alloc_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
									  : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		out_cmd_bfr->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
		VK_ASSERT_SUCCESS(
			vkAllocateCommandBuffers(context->device.logical_device,
									 &alloc_info, &out_cmd_bfr->handle),
			"Failed to allocate command buffer.");
		out_cmd_bfr->state = COMMAND_BUFFER_STATE_IDLE;
}

void vulkan_command_buffer_free(vulkan_context *context, VkCommandPool pool,
								vulkan_command_buffer *cmd_bfr) {
		vkFreeCommandBuffers(context->device.logical_device, pool, 1,
							 &cmd_bfr->handle);
		cmd_bfr->handle = SF_NULL;
		cmd_bfr->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer_begin(vulkan_command_buffer *cmd_bfr, b8 single_use,
								 b8 render_pass_continue, b8 simultaneous_use) {
		VkCommandBufferBeginInfo begin_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		begin_info.flags = 0;
		if (single_use) {
				begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (render_pass_continue) {
				begin_info.flags |=
					VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}
		if (simultaneous_use) {
				begin_info.flags |=
					VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}
		VK_ASSERT_SUCCESS(vkBeginCommandBuffer(cmd_bfr->handle, &begin_info),
						  "Failed to begin command buffer.");
		cmd_bfr->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(vulkan_command_buffer *cmd_bfr) {
		VK_ASSERT_SUCCESS(vkEndCommandBuffer(cmd_bfr->handle),
						  "Failed to end command buffer.");
		cmd_bfr->state = COMMAND_BUFFER_STATE_RECORDING_FINISHED;
}

void vulkan_command_buffer_update_submitted(vulkan_command_buffer *cmd_bfr) {
		// TODO: add more checks
		cmd_bfr->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer *cmd_bfr) {
		// TODO: add more checks
		cmd_bfr->state = COMMAND_BUFFER_STATE_IDLE;
}

void vulkan_command_buffer_alloc_and_begin_single_use(
	vulkan_context *context, VkCommandPool pool,
	vulkan_command_buffer *out_cmd_bfr) {
		vulkan_command_buffer_create(context, pool, TRUE, out_cmd_bfr);
		vulkan_command_buffer_begin(out_cmd_bfr, TRUE, FALSE, FALSE);
}

void vulkan_command_buffer_end_single_use(vulkan_context *context,
										  VkCommandPool pool,
										  vulkan_command_buffer *cmd_bfr,
										  VkQueue queue) {
		vulkan_command_buffer_end(cmd_bfr);

		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_bfr->handle;
		VK_ASSERT_SUCCESS(vkQueueSubmit(queue, 1, &submit_info, SF_NULL),
						  "Failed to submit queue. (single-use variant)");

		// TODO: perhaps a fence instead of this.
		VK_ASSERT_SUCCESS(vkQueueWaitIdle(queue),
						  "Failed to wait for queue to become idle.");
		vulkan_command_buffer_free(context, pool, cmd_bfr);
}
