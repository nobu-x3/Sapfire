#include "vulkan_pipeline.h"
#include "core/sfmemory.h"
#include "math/math_types.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

b8 vulkan_pipeline_create (vulkan_context *context,
						   vulkan_render_pass *render_pass, u32 attribute_count,
						   VkVertexInputAttributeDescription *attributes,
						   u32 descriptor_set_layout_count,
						   VkDescriptorSetLayout *descriptor_set_layouts,
						   u32 stage_count,
						   VkPipelineShaderStageCreateInfo *stages,
						   VkViewport viewport, VkRect2D scissor,
						   b8 is_wireframe, vulkan_pipeline *out_pipeline) {

	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	input_assembly_ci.primitiveRestartEnable = VK_FALSE;
	input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkVertexInputBindingDescription binding_descr;
	binding_descr.binding	= 0;
	binding_descr.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	binding_descr.stride	= sizeof (vertex);

	VkPipelineVertexInputStateCreateInfo vertex_input_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertex_input_ci.vertexBindingDescriptionCount	= 1;
	vertex_input_ci.pVertexBindingDescriptions		= &binding_descr;
	vertex_input_ci.vertexAttributeDescriptionCount = attribute_count;
	vertex_input_ci.pVertexAttributeDescriptions	= attributes;

	VkPipelineRasterizationStateCreateInfo rasterization_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterization_ci.depthClampEnable		 = VK_FALSE;
	rasterization_ci.rasterizerDiscardEnable = VK_FALSE;
	rasterization_ci.polygonMode =
		is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterization_ci.lineWidth				 = 1.0f;
	rasterization_ci.cullMode				 = VK_CULL_MODE_BACK_BIT;
	rasterization_ci.frontFace				 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_ci.depthBiasEnable		 = VK_FALSE;
	rasterization_ci.depthBiasConstantFactor = 0.0f;
	rasterization_ci.depthBiasClamp			 = 0.0f;
	rasterization_ci.depthBiasSlopeFactor	 = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisampling_ci.sampleShadingEnable   = VK_FALSE;
	multisampling_ci.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisampling_ci.minSampleShading	   = 1.0f;
	multisampling_ci.pSampleMask		   = 0;
	multisampling_ci.alphaToCoverageEnable = VK_FALSE;
	multisampling_ci.alphaToOneEnable	   = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depth_stencil.depthTestEnable		= VK_TRUE;
	depth_stencil.depthWriteEnable		= VK_TRUE;
	depth_stencil.depthCompareOp		= VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable		= VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment_state;
	sfmemset (&color_blend_attachment_state, 0,
			  sizeof (VkPipelineColorBlendAttachmentState));
	color_blend_attachment_state.blendEnable = VK_TRUE;
	color_blend_attachment_state.srcColorBlendFactor =
		VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstColorBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor =
		VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	color_blend_attachment_state.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	color_blend_state_ci.logicOpEnable	 = VK_FALSE;
	color_blend_state_ci.logicOp		 = VK_LOGIC_OP_COPY;
	color_blend_state_ci.attachmentCount = 1;
	color_blend_state_ci.pAttachments	 = &color_blend_attachment_state;

	VkDynamicState dynamic_states[3] = {VK_DYNAMIC_STATE_VIEWPORT,
										VK_DYNAMIC_STATE_SCISSOR,
										VK_DYNAMIC_STATE_LINE_WIDTH};

	VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	dynamic_state_ci.dynamicStateCount = 3;
	dynamic_state_ci.pDynamicStates	   = dynamic_states;

	VkPipelineViewportStateCreateInfo viewport_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewport_ci.viewportCount = 1;
	viewport_ci.pViewports	  = &viewport;
	viewport_ci.scissorCount  = 1;
	viewport_ci.pScissors	  = &scissor;

	VkPipelineLayoutCreateInfo pipeline_layout_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	// Descriptor set layouts
	pipeline_layout_ci.setLayoutCount = descriptor_set_layout_count;
	pipeline_layout_ci.pSetLayouts	  = descriptor_set_layouts;

	// Push constants
	VkPushConstantRange push_const;
	push_const.offset	  = 0;
	push_const.size		  = sizeof (mat4) * 2; // 128 bytes min in most cards
	push_const.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipeline_layout_ci.pushConstantRangeCount = 1;
	pipeline_layout_ci.pPushConstantRanges	  = &push_const;

	VK_ASSERT_SUCCESS (vkCreatePipelineLayout (
						   context->device.logical_device, &pipeline_layout_ci,
						   context->allocator, &out_pipeline->layout),
					   "Failed to create pipeline layout");

	VkGraphicsPipelineCreateInfo pipeline_ci = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipeline_ci.stageCount			= stage_count;
	pipeline_ci.pStages				= stages;
	pipeline_ci.pVertexInputState	= &vertex_input_ci;
	pipeline_ci.pInputAssemblyState = &input_assembly_ci;

	pipeline_ci.pViewportState		= &viewport_ci;
	pipeline_ci.pRasterizationState = &rasterization_ci;
	pipeline_ci.pMultisampleState	= &multisampling_ci;
	pipeline_ci.pDepthStencilState	= &depth_stencil;
	pipeline_ci.pColorBlendState	= &color_blend_state_ci;
	pipeline_ci.pDynamicState		= &dynamic_state_ci;
	pipeline_ci.pTessellationState	= 0;

	pipeline_ci.layout = out_pipeline->layout;

	pipeline_ci.renderPass		   = render_pass->handle;
	pipeline_ci.subpass			   = 0;
	pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_ci.basePipelineIndex  = -1;

	VK_ASSERT_SUCCESS (
		vkCreateGraphicsPipelines (context->device.logical_device,
								   VK_NULL_HANDLE, 1, &pipeline_ci,
								   context->allocator, &out_pipeline->handle),
		"Failed to create graphics pipeline.");
	return TRUE;
}

void vulkan_pipeline_destroy (vulkan_context *context,
							  vulkan_pipeline *pipeline) {
	if (pipeline) {
		// Destroy pipeline
		if (pipeline->handle) {
			vkDestroyPipeline (context->device.logical_device, pipeline->handle,
							   context->allocator);
			pipeline->handle = SF_NULL;
		}

		// Destroy layout
		if (pipeline->layout) {
			vkDestroyPipelineLayout (context->device.logical_device,
									 pipeline->layout, context->allocator);
			pipeline->layout = SF_NULL;
		}
	}
}

void vulkan_pipeline_bind (vulkan_command_buffer *command_buffer,
						   VkPipelineBindPoint bind_point,
						   vulkan_pipeline *pipeline) {
	vkCmdBindPipeline (command_buffer->handle, bind_point, pipeline->handle);
}