#include "vulkan_shader.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "defines.h"
#include "math/math_types.h"
#include "math/sfmath.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_shader_module.h"
#include "renderer/vulkan/vulkan_types.h"
#include "resources/resource_types.h"
#include "vulkan/vulkan_core.h"

#define BUILTIN_SHADER_NAME "shader_builtin"

b8 vulkan_shader_create (vulkan_context *context, vulkan_shader *out_shader) {
	char stage_type_strings[SHADER_STAGE_COUNT][5]		  = {"vert", "frag"};
	VkShaderStageFlagBits stage_types[SHADER_STAGE_COUNT] = {
		VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
	for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
		if (!create_shader_module (context, BUILTIN_SHADER_NAME,
								   stage_type_strings[i], stage_types[i], i,
								   out_shader->stages)) {
			SF_FATAL ("Failed to create shader module %s for %s.",
					  stage_type_strings[i], BUILTIN_SHADER_NAME);
			return FALSE;
		}
	}

	// Descriptors
	VkDescriptorSetLayoutBinding scene_desc_layout_binding;
	scene_desc_layout_binding.descriptorCount = 1;
	scene_desc_layout_binding.descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	scene_desc_layout_binding.pImmutableSamplers = SF_NULL;
	scene_desc_layout_binding.binding			 = 0;
	scene_desc_layout_binding.stageFlags		 = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo scene_descr_set_ci = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	scene_descr_set_ci.bindingCount = 1;
	scene_descr_set_ci.pBindings	= &scene_desc_layout_binding;
	VK_ASSERT_SUCCESS (
		vkCreateDescriptorSetLayout (context->device.logical_device,
									 &scene_descr_set_ci, context->allocator,
									 &out_shader->scene_descriptor_set_layout),
		"Failed to create descriptor set layout.");

	VkDescriptorPoolSize scene_pool_size;
	scene_pool_size.descriptorCount = context->swapchain.image_count;
	scene_pool_size.type			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo scene_pool_ci = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	scene_pool_ci.poolSizeCount = 1;
	scene_pool_ci.pPoolSizes	= &scene_pool_size;
	scene_pool_ci.maxSets		= context->swapchain.image_count;
	VK_ASSERT_SUCCESS (
		vkCreateDescriptorPool (context->device.logical_device, &scene_pool_ci,
								context->allocator,
								&out_shader->scene_descriptor_pool),
		"Failed to create descriptor pool.");
	const u32 local_sampler_count											= 1;
	VkDescriptorType local_descriptor_types[VULKAN_SHADER_DESCRIPTOR_COUNT] = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
	VkDescriptorSetLayoutBinding local_bindings[VULKAN_SHADER_DESCRIPTOR_COUNT];
	sfmemset (&local_bindings, 0,
			  sizeof (VkDescriptorSetLayoutBinding) *
				  VULKAN_SHADER_DESCRIPTOR_COUNT);
	for (u32 i = 0; i < VULKAN_SHADER_DESCRIPTOR_COUNT; ++i) {
		local_bindings[i].binding		  = i;
		local_bindings[i].stageFlags	  = VK_SHADER_STAGE_FRAGMENT_BIT;
		local_bindings[i].descriptorCount = 1;
		local_bindings[i].descriptorType  = local_descriptor_types[i];
	}
	VkDescriptorSetLayoutCreateInfo local_layout_ci = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	local_layout_ci.bindingCount = VULKAN_SHADER_DESCRIPTOR_COUNT;
	local_layout_ci.pBindings	 = local_bindings;
	VK_ASSERT_SUCCESS (
		vkCreateDescriptorSetLayout (context->device.logical_device,
									 &local_layout_ci, context->allocator,
									 &out_shader->mesh_descriptor_set_layout),
		"Failed to create mesh descriptor set layout.");
	VkDescriptorPoolSize local_pool_size[VULKAN_SHADER_DESCRIPTOR_COUNT];
	local_pool_size[0].type			   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	local_pool_size[0].descriptorCount = VULKAN_MAX_MESH_COUNT;
	local_pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	local_pool_size[1].descriptorCount					= VULKAN_MAX_MESH_COUNT * local_sampler_count;
	VkDescriptorPoolCreateInfo local_descriptor_pool_ci = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	local_descriptor_pool_ci.poolSizeCount = 2;
	local_descriptor_pool_ci.pPoolSizes	   = local_pool_size;
	local_descriptor_pool_ci.maxSets	   = VULKAN_MAX_MESH_COUNT;
	VK_ASSERT_SUCCESS (
		vkCreateDescriptorPool (context->device.logical_device,
								&local_descriptor_pool_ci, context->allocator,
								&out_shader->mesh_descriptor_pool),
		"Failed to create mesh descriptor pool.");
	VkViewport viewport;
	viewport.x		  = 0.0f;
	viewport.y		  = 0.0;
	viewport.width	  = (f32)context->framebuffer_width;
	viewport.height	  = (f32)context->framebuffer_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset.x	  = 0;
	scissor.offset.y	  = 0;
	scissor.extent.width  = context->framebuffer_width;
	scissor.extent.height = context->framebuffer_height;
	// TODO: config
#define ATTRIBUTE_COUNT 2
	u32 offset = 0;
	VkVertexInputAttributeDescription
		attributeDescription[ATTRIBUTE_COUNT];
	VkFormat formats[ATTRIBUTE_COUNT] = {
		VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT};
	u64 sizes[ATTRIBUTE_COUNT] = {sizeof (vec3), sizeof (vec2)};
	for (u32 i = 0; i < ATTRIBUTE_COUNT; ++i) {
		attributeDescription[i].binding	 = 0;
		attributeDescription[i].location = i;
		attributeDescription[i].format	 = formats[i];
		attributeDescription[i].offset	 = offset;
		offset += sizes[i];
	}

    const u32 description_set_layout_count = 2;
	VkDescriptorSetLayout layouts[2] = {
		out_shader->scene_descriptor_set_layout,
    out_shader->mesh_descriptor_set_layout};

	VkPipelineShaderStageCreateInfo stage_ci[SHADER_STAGE_COUNT];
	sfmemset (stage_ci, 0, sizeof (stage_ci));
	for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
		stage_ci[i].sType =
			out_shader->stages[i].shader_stage_create_info.sType;
		stage_ci[i] = out_shader->stages[i].shader_stage_create_info;
	}
	if (!vulkan_pipeline_create (context, &context->main_render_pass, ATTRIBUTE_COUNT,
								 attributeDescription, description_set_layout_count, layouts,
								 SHADER_STAGE_COUNT, stage_ci, viewport,
								 scissor, FALSE, &out_shader->pipeline)) {
		SF_FATAL ("Failed to load graphics pipeline for shader.");
		return FALSE;
	}

	if (!vulkan_buffer_create (context, sizeof (scene_uniform),
							   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
								   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
								   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							   &out_shader->scene_uniform_buffer)) {
		SF_FATAL ("Failed to create uniform buffer.");
		return FALSE;
	}
	vulkan_buffer_bind (context, &out_shader->scene_uniform_buffer, 0);
	// for allocation
	VkDescriptorSetLayout layouts_alloc[8] = {
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout,
		out_shader->scene_descriptor_set_layout};

	VkDescriptorSetAllocateInfo desc_alloc_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	desc_alloc_info.descriptorPool	   = out_shader->scene_descriptor_pool;
	desc_alloc_info.descriptorSetCount = context->swapchain.image_count;
	desc_alloc_info.pSetLayouts		   = layouts_alloc;
	VK_ASSERT_SUCCESS (vkAllocateDescriptorSets (
						   context->device.logical_device, &desc_alloc_info,
						   out_shader->scene_descriptor_sets),
					   "Failed to allocate descriptor sets.");

	if (!vulkan_buffer_create (context, sizeof (mesh_uniform),
							   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
								   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
								   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							   &out_shader->mesh_uniform_buffer)) {
		SF_FATAL ("Failed to create mesh data uniform for shader.");
		return FALSE;
	}
    vulkan_buffer_bind(context, &out_shader->mesh_uniform_buffer, 0);
	return TRUE;
}

void vulkan_shader_destroy (vulkan_context *context, vulkan_shader *shader) {
	vulkan_buffer_destroy (context, &shader->scene_uniform_buffer);
	vulkan_buffer_destroy (context, &shader->mesh_uniform_buffer);
    vulkan_pipeline_destroy (context, &shader->pipeline);
	vkDestroyDescriptorPool (context->device.logical_device,
							 shader->scene_descriptor_pool, context->allocator);
	vkDestroyDescriptorSetLayout (context->device.logical_device,
								  shader->scene_descriptor_set_layout,
								  context->allocator);
   vkDestroyDescriptorPool(context->device.logical_device, shader->mesh_descriptor_pool, context->allocator);
   vkDestroyDescriptorSetLayout(context->device.logical_device,shader->mesh_descriptor_set_layout, context->allocator);
	for (int i = 0; i < SHADER_STAGE_COUNT; ++i) {
		vkDestroyShaderModule (context->device.logical_device,
							   shader->stages[i].handle, context->allocator);
		shader->stages[i].handle = SF_NULL;
	}
}

void vulkan_shader_update_uniforms (vulkan_context *context,
									vulkan_shader *shader,
									scene_uniform *data) {
	i32 img_index = context->image_index;
	VkCommandBuffer cmd_bfr =
		context->graphics_command_buffers[img_index].handle;
	VkDescriptorSet descr_set = shader->scene_descriptor_sets[img_index];

	u32 size   = sizeof (scene_uniform);
	u64 offset = 0;
	vulkan_buffer_load_data (context, &shader->scene_uniform_buffer, size,
							 offset, 0, data);
	VkDescriptorBufferInfo descr_info;
	descr_info.buffer = shader->scene_uniform_buffer.handle;
	descr_info.offset = offset;
	descr_info.range  = size;

	VkWriteDescriptorSet write_descr = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write_descr.descriptorCount		 = 1;
	write_descr.descriptorType		 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descr.dstBinding			 = 0;
	write_descr.dstArrayElement		 = 0;
	write_descr.dstSet				 = descr_set;
	write_descr.pBufferInfo			 = &descr_info;
	vkUpdateDescriptorSets (context->device.logical_device, 1, &write_descr, 0,
							0);

	vkCmdBindDescriptorSets (cmd_bfr, VK_PIPELINE_BIND_POINT_GRAPHICS,
							 shader->pipeline.layout, 0, 1, &descr_set, 0,
							 SF_NULL);
}

void vulkan_shader_update_model (vulkan_context *context, vulkan_shader *shader,
								 mesh_data data) {
	i32 img_index = context->image_index;
	VkCommandBuffer cmd_bfr =
		context->graphics_command_buffers[img_index].handle;
	vkCmdPushConstants (cmd_bfr, shader->pipeline.layout,
						VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof (mat4),
						&data.model);
    // material data
    vulkan_shader_mesh_state* state = &shader->mesh_states[data.id];
    VkDescriptorSet mesh_descriptor_set = state->descriptor_sets[img_index];
    VkWriteDescriptorSet descriptor_writes[VULKAN_SHADER_DESCRIPTOR_COUNT];
    sfmemset(descriptor_writes, 0, sizeof(VkWriteDescriptorSet) * VULKAN_SHADER_DESCRIPTOR_COUNT);
    u32 desc_count = 0;
    u32 desc_index = 0;
    // Desc 0 - uniform buffer
    u32 range = sizeof(mesh_uniform);
    u64 offset = sizeof(mesh_uniform) * data.id;
    mesh_uniform ubo;
    sfmemset(&ubo, 0, sizeof(mesh_uniform));
    ubo.diffuse_color = vec4_create(1.0f,1.0f,1.0f,1.0f);
    static f32 accumulator = 0.0f;
    accumulator += 0.01;
    f32 s = (sfsin(accumulator) + 1.0f) / 2.0f;  // scale from -1, 1 to 0, 1
    ubo.diffuse_color = vec4_create(s, s, s, 1.0f);
    vulkan_buffer_load_data(context, &shader->mesh_uniform_buffer, range, offset, 0, &ubo);

    if(state->descriptor_states[desc_index].generations[img_index] == INVALID_ID){
        VkDescriptorBufferInfo buf_info;
        buf_info.buffer = shader->mesh_uniform_buffer.handle;
        buf_info.range = range;
        buf_info.offset = offset;
        VkWriteDescriptorSet desc = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        desc.descriptorCount = 1;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc.pBufferInfo = &buf_info;
        desc.dstSet = mesh_descriptor_set;
        desc.dstBinding = desc_index;
        descriptor_writes[desc_count] = desc;
        desc_count++;
        state->descriptor_states[desc_index].generations[img_index]  = 1;
    }
    desc_index++;
#define SAMPLER_COUNT   1
    VkDescriptorImageInfo image_infos[SAMPLER_COUNT];
    for(u32 s_ind = 0; s_ind < SAMPLER_COUNT; ++s_ind){
        texture* t = data.textures[s_ind];
        u32* generation = &state->descriptor_states[desc_index].generations[img_index];
        if(t && (*generation != t->generation || *generation == INVALID_ID)){
            vulkan_texture_data* internal = (vulkan_texture_data*)t->data;
            image_infos[s_ind].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos[s_ind].imageView = internal->image.view;
            image_infos[s_ind].sampler = internal->sampler;
            VkWriteDescriptorSet desc = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            desc.descriptorCount = 1;
            desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            desc.pImageInfo = &image_infos[s_ind];
            desc.dstSet = mesh_descriptor_set;
            desc.dstBinding = desc_index;
            descriptor_writes[desc_count]  = desc;
            desc_count++;
            if(t->generation != INVALID_ID){
                *generation = t->generation;
            }
            desc_index++;
        }
    }
    if(desc_count > 0){
        vkUpdateDescriptorSets(context->device.logical_device, desc_count, descriptor_writes, 0, SF_NULL);
    }
    vkCmdBindDescriptorSets(cmd_bfr, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout, 1, 1, &mesh_descriptor_set, 0, SF_NULL);
}

void vulkan_shader_bind (vulkan_context *context, vulkan_shader *shader) {
	u32 img_index = context->image_index;
	vulkan_pipeline_bind (&context->graphics_command_buffers[img_index],
						  VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

b8 vulkan_shader_alloc(vulkan_context *context, vulkan_shader* shader, u32* out_id){
    // TODO: potentially ref count
    *out_id = shader->mesh_uniform_buffer_index;
    u32 id = *out_id;
    u8 sets_count = context->swapchain.image_count;
    shader->mesh_uniform_buffer_index++;
    vulkan_shader_mesh_state* state = &shader->mesh_states[id];
    for(u32 i = 0; i < VULKAN_SHADER_DESCRIPTOR_COUNT; ++i){
        for(u32 j = 0; j < sets_count; ++j){
            state->descriptor_states[i].generations[j] = INVALID_ID;
        }
    }
    VkDescriptorSetLayout layouts[8] = {
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
        shader->mesh_descriptor_set_layout,
    };
    VkDescriptorSetAllocateInfo desc_set_ci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    desc_set_ci.descriptorSetCount  =   sets_count;
    desc_set_ci.pSetLayouts = layouts;
    desc_set_ci.descriptorPool = shader->mesh_descriptor_pool;
    VK_ASSERT_SUCCESS(vkAllocateDescriptorSets(context->device.logical_device,&desc_set_ci,state->descriptor_sets), "Failed to allocate descriptor sets.");
    return TRUE;
}

void vulkan_shader_free(vulkan_context* context, vulkan_shader* shader, u32 id){
    vulkan_shader_mesh_state* state = &shader->mesh_states[id];
    u32 sets_count = context->swapchain.image_count;
    VK_ASSERT_SUCCESS(vkFreeDescriptorSets(context->device.logical_device, shader->mesh_descriptor_pool, sets_count, state->descriptor_sets), "Failed to free mesh descriptor sets.");
    for(u32 i = 0; i < VULKAN_SHADER_DESCRIPTOR_COUNT; ++i){
        for(u32 j = 0; j < sets_count; ++j){
            state->descriptor_states[i].generations[j] = INVALID_ID;
        }
    }
    // TODO: ref counting stuff
}
