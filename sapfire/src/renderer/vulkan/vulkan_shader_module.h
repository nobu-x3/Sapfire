#pragma once

#include "vulkan_types.h"

b8 create_shader_module (vulkan_context *context, const char *name,
						 const char *type_str, VkShaderStageFlagBits flag_bits,
						 u32 stage, vulkan_shader_stage *shader_stages);