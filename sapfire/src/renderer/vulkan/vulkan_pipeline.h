#pragma once

#include "vulkan_types.h"

b8 vulkan_pipeline_create (vulkan_context* context,
						   vulkan_render_pass* render_pass, u32 attribute_count,
						   VkVertexInputAttributeDescription* attributes,
						   u32 descriptor_set_layout_count,
						   VkDescriptorSetLayout* descriptor_set_layouts,
						   u32 stage_count,
						   VkPipelineShaderStageCreateInfo* stages,
						   VkViewport viewport, VkRect2D scissor,
						   b8 is_wireframe, vulkan_pipeline* out_pipeline);

void vulkan_pipeline_destroy (vulkan_context* context,
							  vulkan_pipeline* pipeline);
void vulkan_pipeline_bind (vulkan_command_buffer* command_buffer,
						   VkPipelineBindPoint bind_point,
						   vulkan_pipeline* pipeline);
