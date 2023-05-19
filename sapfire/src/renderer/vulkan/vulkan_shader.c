#include "core/logger.h"
#include "core/sfmemory.h"
#include "math/math_types.h"
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
		VkPipelineShaderStageCreateInfo stage_ci[SHADER_STAGE_COUNT];
		sfmemset(stage_ci, 0, sizeof(stage_ci));
		for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
				stage_ci[i].sType =
					out_shader->stages[i].shader_stage_create_info.sType;
				stage_ci[i] = out_shader->stages[i].shader_stage_create_info;
		}
		if (!vulkan_pipeline_create(context, &context->main_render_pass, 1,
									attributeDescription, 0, SF_NULL,
									SHADER_STAGE_COUNT, stage_ci, viewport,
									scissor, FALSE, &out_shader->pipeline)) {
				SF_FATAL("Failed to load graphics pipeline for shader.");
				return FALSE;
		}
		return TRUE;
}

void vulkan_shader_destroy(vulkan_context *context, vulkan_shader *shader) {
		vulkan_pipeline_destroy(context, &shader->pipeline);
		for (int i = 0; i < SHADER_STAGE_COUNT; ++i) {
				vkDestroyShaderModule(context->device.logical_device,
									  shader->stages[i].handle,
									  context->allocator);
				shader->stages[i].handle = SF_NULL;
		}
}

void vulkan_shader_bind(vulkan_context *context, vulkan_shader *shader) {}
