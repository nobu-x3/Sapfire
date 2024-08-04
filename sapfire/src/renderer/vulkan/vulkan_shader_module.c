#include "vulkan_shader_module.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "core/sfstring.h"
#include "platform/filesystem.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

b8 create_shader_module (vulkan_context *context, const char *name,
						 const char *type_str,
						 VkShaderStageFlagBits stage_flags, u32 stage,
						 vulkan_shader_stage *shader_stages) {
	char file_name[256];
	sfstrfmt (file_name, "assets/shaders/%s.%s.spv", name, type_str);
	sfmemset (&shader_stages[stage].create_info, 0,
			  sizeof (VkShaderModuleCreateInfo));
	sfmemset (&shader_stages[stage].shader_stage_create_info, 0,
			  sizeof (VkPipelineShaderStageCreateInfo));
	shader_stages[stage].create_info.sType =
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	file_handle handle;
	if (!filesystem_open (file_name, FILE_MODE_READ, TRUE, &handle)) {
		SF_ERROR ("Unable to read shader module %s", file_name);
		return FALSE;
	}
	u64 size = 0;
	u8 *buf	 = SF_NULL;
	if (!filesystem_read_all_bytes (&handle, &buf, &size)) {
		SF_ERROR ("Unable to read (binary) module: %s", file_name);
		return FALSE;
	}
	shader_stages[stage].create_info.codeSize = size;
	shader_stages[stage].create_info.pCode	  = (u32 *)buf;
	filesystem_close (&handle);
	VK_ASSERT_SUCCESS (vkCreateShaderModule (context->device.logical_device,
											 &shader_stages[stage].create_info,
											 context->allocator,
											 &shader_stages[stage].handle),
					   "Failed to create shader module!");
	shader_stages[stage].shader_stage_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[stage].shader_stage_create_info.pName = "main";
	shader_stages[stage].shader_stage_create_info.stage = stage_flags;
	shader_stages[stage].shader_stage_create_info.module =
		shader_stages[stage].handle;
	if (buf) {
		sffree (buf, sizeof (u8) * size, MEMORY_TAG_STRING);
		buf = SF_NULL;
	}
	return TRUE;
}