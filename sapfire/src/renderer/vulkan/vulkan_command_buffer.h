#pragma once

#include "vulkan_types.h"

void vulkan_command_buffer_create (vulkan_context* context, VkCommandPool pool,
								   b8 is_primary,
								   vulkan_command_buffer* out_cmd_bfr);
void vulkan_command_buffer_free (vulkan_context* context, VkCommandPool pool,
								 vulkan_command_buffer* cmd_bfr);
void vulkan_command_buffer_begin (vulkan_command_buffer* cmd_bfr, b8 single_use,
								  b8 render_pass_continue, b8 simultaneous_use);
void vulkan_command_buffer_end (vulkan_command_buffer* cmd_bfr);
void vulkan_command_buffer_update_submitted (vulkan_command_buffer* cmd_bfr);
void vulkan_command_buffer_reset (vulkan_command_buffer* cmd_bfr);

// Allocates and begins recording.
void vulkan_command_buffer_alloc_and_begin_single_use (
	vulkan_context* context, VkCommandPool pool,
	vulkan_command_buffer* out_cmd_bfr);

// Ends recording, submits to queue, waits for queue operation and 'frees'.
void vulkan_command_buffer_end_single_use (vulkan_context* context,
										   VkCommandPool pool,
										   vulkan_command_buffer* cmd_bfr,
										   VkQueue queue);
