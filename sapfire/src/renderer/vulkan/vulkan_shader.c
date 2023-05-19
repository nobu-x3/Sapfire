#include "core/logger.h"
#include "renderer/vulkan/vulkan_shader_module.h"
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
		return TRUE;
}

void vulkan_shader_destroy(vulkan_context *context, vulkan_shader *shader) {
		for (int i = 0; i < SHADER_STAGE_COUNT; ++i) {
				vkDestroyShaderModule(context->device.logical_device,
									  shader->stages[i].handle,
									  context->allocator);
				shader->stages[i].handle = SF_NULL;
		}
}

void vulkan_shader_bind(vulkan_context *context, vulkan_shader *shader) {}
