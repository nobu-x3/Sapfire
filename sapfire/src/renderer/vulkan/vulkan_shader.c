#include "core/logger.h"
#include "core/sfmemory.h"
#include "math/math_types.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_shader_module.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_shader.h"

#define BUILTIN_SHADER_NAME "shader_builtin"

b8 vulkan_shader_create(vulkan_context *context, vulkan_shader *out_shader) {
		char stage_type_strings[SHADER_STAGE_COUNT][5] = {"vert", "frag"};
		VkShaderStageFlagBits stage_types[SHADER_STAGE_COUNT] = {
			VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
		for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
				if (!create_shader_module(context, BUILTIN_SHADER_NAME,
										  stage_type_strings[i], stage_types[i],
										  i, out_shader->stages)) {
						SF_FATAL("Failed to create shader module %s for %s.",
								 stage_type_strings[i], BUILTIN_SHADER_NAME);
						return FALSE;
				}
		}

		// Descriptors
		VkDescriptorSetLayoutBinding desc_layout_binding;
		desc_layout_binding.descriptorCount = 1;
		desc_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		desc_layout_binding.pImmutableSamplers = SF_NULL;
		desc_layout_binding.binding = 0;
		desc_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo descr_set_ci = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		descr_set_ci.bindingCount = 1;
		descr_set_ci.pBindings = &desc_layout_binding;
		VK_ASSERT_SUCCESS(
			vkCreateDescriptorSetLayout(context->device.logical_device,
										&descr_set_ci, context->allocator,
										&out_shader->descriptor_set_layout),
			"Failed to create descriptor set layout.");

		VkDescriptorPoolSize pool_size;
		pool_size.descriptorCount = context->swapchain.image_count;
		pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo pool_ci = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		pool_ci.poolSizeCount = 1;
		pool_ci.pPoolSizes = &pool_size;
		pool_ci.maxSets = context->swapchain.image_count;
		VK_ASSERT_SUCCESS(vkCreateDescriptorPool(context->device.logical_device,
												 &pool_ci, context->allocator,
												 &out_shader->descriptor_pool),
						  "Failed to create descriptor pool.");

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0;
		viewport.width = (f32)context->framebuffer_width;
		viewport.height = (f32)context->framebuffer_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = context->framebuffer_width;
		scissor.extent.height = context->framebuffer_height;

		// TODO: config, loop
		u32 offset = 0;
		VkVertexInputAttributeDescription attributeDescription[1];
		VkFormat formats[1] = {VK_FORMAT_R32G32B32_SFLOAT};
		u64 sizes[1] = {sizeof(vec3)};
		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = formats[0];
		attributeDescription[0].offset = offset;
		offset += sizes[0];

		VkDescriptorSetLayout layouts[1] = {out_shader->descriptor_set_layout};

		VkPipelineShaderStageCreateInfo stage_ci[SHADER_STAGE_COUNT];
		sfmemset(stage_ci, 0, sizeof(stage_ci));
		for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
				stage_ci[i].sType =
					out_shader->stages[i].shader_stage_create_info.sType;
				stage_ci[i] = out_shader->stages[i].shader_stage_create_info;
		}
		if (!vulkan_pipeline_create(context, &context->main_render_pass, 1,
									attributeDescription, 1, layouts,
									SHADER_STAGE_COUNT, stage_ci, viewport,
									scissor, FALSE, &out_shader->pipeline)) {
				SF_FATAL("Failed to load graphics pipeline for shader.");
				return FALSE;
		}

		if (!vulkan_buffer_create(context, sizeof(scene_data),
								  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
									  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								  &out_shader->uniform_buffer)) {
				SF_FATAL("Failed to create uniform buffer.");
				return FALSE;
		}
		vulkan_buffer_bind(context, &out_shader->uniform_buffer, 0);
		// for allocation
		VkDescriptorSetLayout layouts_alloc[8] = {
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout,
			out_shader->descriptor_set_layout};

		VkDescriptorSetAllocateInfo desc_alloc_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		desc_alloc_info.descriptorPool = out_shader->descriptor_pool;
		desc_alloc_info.descriptorSetCount = context->swapchain.image_count;
		desc_alloc_info.pSetLayouts = layouts_alloc;
		VK_ASSERT_SUCCESS(vkAllocateDescriptorSets(
							  context->device.logical_device, &desc_alloc_info,
							  out_shader->descriptor_sets),
						  "Failed to allocate descriptor sets.");

		return TRUE;
}

void vulkan_shader_destroy(vulkan_context *context, vulkan_shader *shader) {
		vulkan_buffer_destroy(context, &shader->uniform_buffer);
		vulkan_pipeline_destroy(context, &shader->pipeline);
		vkDestroyDescriptorPool(context->device.logical_device,
								shader->descriptor_pool, context->allocator);
		vkDestroyDescriptorSetLayout(context->device.logical_device,
									 shader->descriptor_set_layout,
									 context->allocator);
		for (int i = 0; i < SHADER_STAGE_COUNT; ++i) {
				vkDestroyShaderModule(context->device.logical_device,
									  shader->stages[i].handle,
									  context->allocator);
				shader->stages[i].handle = SF_NULL;
		}
}

void vulkan_shader_update_uniforms(vulkan_context *context,
								   vulkan_shader *shader, scene_data *data) {
		i32 img_index = context->image_index;
		VkCommandBuffer cmd_bfr =
			context->graphics_command_buffers[img_index].handle;
		VkDescriptorSet descr_set = shader->descriptor_sets[img_index];

		u32 size = sizeof(scene_data);
		u64 offset = 0;
		vulkan_buffer_load_data(context, &shader->uniform_buffer, size, offset,
								0, data);
		VkDescriptorBufferInfo descr_info;
		descr_info.buffer = shader->uniform_buffer.handle;
		descr_info.offset = offset;
		descr_info.range = size;

		VkWriteDescriptorSet write_descr = {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write_descr.descriptorCount = 1;
		write_descr.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_descr.dstBinding = 0;
		write_descr.dstArrayElement = 0;
		write_descr.dstSet = descr_set;
		write_descr.pBufferInfo = &descr_info;
		vkUpdateDescriptorSets(context->device.logical_device, 1, &write_descr,
							   0, 0);

		vkCmdBindDescriptorSets(cmd_bfr, VK_PIPELINE_BIND_POINT_GRAPHICS,
								shader->pipeline.layout, 0, 1, &descr_set, 0,
								SF_NULL);
}

void vulkan_shader_bind(vulkan_context *context, vulkan_shader *shader) {
		u32 img_index = context->image_index;
		vulkan_pipeline_bind(&context->graphics_command_buffers[img_index],
							 VK_PIPELINE_BIND_POINT_GRAPHICS,
							 &shader->pipeline);
}
