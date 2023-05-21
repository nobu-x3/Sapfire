#include "containers/vector.h"
#include "core/asserts.h"
#include "core/event.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "core/sfstring.h"
#include "defines.h"
#include "math/math_types.h"
#include "platform/platform.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_device.h"
#include "renderer/vulkan/vulkan_fence.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_render_pass.h"
#include "renderer/vulkan/vulkan_shader.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "vulkan_platform.h"
#include "vulkan_provider.h"
#include "vulkan_types.h"
#include <stdint.h>
#include <string.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#define DEBUG
// TODO: really think about singletons...
static vulkan_context context;
static u32 cached_window_width = 0;
static u32 cached_window_height = 0;

i32 find_memory_index(u32 type_filter, u32 property_flags);

void recreate_frambuffers(vulkan_swapchain *swapchain,
						  vulkan_render_pass *render_pass);

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	unsigned int message_types,
	const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

b8 window_resized(u16 code, void *sender, void *listener_list,
				  event_context data);

b8 recreate_swapchain();

void create_command_buffers();

b8 create_buffers(vulkan_context *context); // TODO: remove this

b8 vulkan_initialize(renderer_provider *api, const char *app_name,
					 struct platform_state *plat_state) {
		event_register(EVENT_CODE_WINDOW_RESIZED, &context, window_resized);
		context.find_memory_index = find_memory_index;
		// TODO: config
		extent2d extent_window = platform_get_drawable_extent(plat_state);
		context.framebuffer_width = extent_window.w;
		context.framebuffer_height = extent_window.h;
		// TODO: implement custom allocators.
		context.allocator = SF_NULL;
		VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
		app_info.apiVersion = VK_API_VERSION_1_3;
		app_info.pApplicationName = app_name;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Sapfire Engine";
		const char **ext_names = vector_create(const char *);
		platform_get_required_extension_names(plat_state, &ext_names);
#if defined(DEBUG)
		vector_push(ext_names, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		SF_DEBUG("Required extensions:");
		u32 length = vector_len(ext_names);
		for (u32 i = 0; i < length; ++i) {
				SF_DEBUG(ext_names[i]);
		}
#endif
		u32 ext_count = vector_len(ext_names);
		u32 valid_count = 0;
		const char **valid_names = SF_NULL;

#if defined(DEBUG)
		valid_names = vector_create(const char *);
		vector_push(valid_names, &"VK_LAYER_KHRONOS_validation");
		valid_count = vector_len(valid_names);
		// available validation layers
		u32 available_count = 0;
		VK_ASSERT_SUCCESS(
			vkEnumerateInstanceLayerProperties(&available_count, SF_NULL),
			"Failed to enumerate instance layer properties!");
		VkLayerProperties *available_properties =
			vector_reserve(VkLayerProperties, available_count);
		VK_ASSERT_SUCCESS(vkEnumerateInstanceLayerProperties(
							  &available_count, available_properties),
						  "Failed to enumerate instance layer properties!");
		for (u32 i = 0; i < valid_count; ++i) {
				SF_DEBUG("Searching for layer %s.", valid_names[i]);
				b8 found = FALSE;
				for (u32 j = 0; j < available_count; ++j) {
						if (sfstreq(valid_names[i],
									available_properties[j].layerName)) {
								found = TRUE;
								SF_DEBUG("Success!");
								break;
						}
				}
				if (!found) {
						SF_FATAL(
							"Failure. Required validation layer %s is missing",
							valid_names[i]);
						return FALSE;
				}
		}
		vector_destroy(available_properties);
#endif
		VkInstanceCreateInfo create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = ext_count;
		create_info.ppEnabledExtensionNames = ext_names;
		create_info.enabledLayerCount = valid_count;
		create_info.ppEnabledLayerNames = valid_names;

		VK_ASSERT_SUCCESS(vkCreateInstance(&create_info, context.allocator,
										   &context.instance),
						  "Failed to create vkInstance!");
		SF_INFO("Vulkan instance created.");
		vector_destroy(valid_names);
		vector_destroy(ext_names);

#if defined(DEBUG)
		i32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		// | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT // This feeds out ALL
		// the info | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT // SUPER
		// verbose, use at your own risk
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
		debug_create_info.messageSeverity = log_severity;
		debug_create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = vk_debug_callback;
		PFN_vkCreateDebugUtilsMessengerEXT func =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				context.instance, "vkCreateDebugUtilsMessengerEXT");
		SF_ASSERT(func, "Failed to create debug messenger!");
		VK_ASSERT_SUCCESS(func(context.instance, &debug_create_info,
							   context.allocator, &context.debug_messenger),
						  "Failed to create vulkan debugger!");
		SF_DEBUG("Vulkan debugger created.");

#endif
		if (!platform_create_vulkan_surface(plat_state, &context)) {
				SF_FATAL("Failed to create vulkan surface.");
				return FALSE;
		}
		SF_INFO("Vulkan surface created.");
		if (!vulkan_device_create(&context)) {
				SF_FATAL("Failed to create vulkan devices.");
				return FALSE;
		}

		vulkan_swapchain_create(&context, context.framebuffer_width,
								context.framebuffer_height, &context.swapchain,
								SF_NULL);

		color color = {0.3f, 0.5f, 0.7f, 1.0f};
		extent2d extent = {0.0f, 0.0f, context.framebuffer_width,
						   context.framebuffer_height};
		vulkan_render_pass_create(&context, color, extent_window, 1.0f, 0,
								  &context.main_render_pass);

		context.swapchain.framebuffers =
			vector_reserve(vulkan_framebuffer, context.swapchain.image_count);
		recreate_frambuffers(&context.swapchain, &context.main_render_pass);

		create_command_buffers();

		context.image_available_semaphores =
			vector_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
		context.queue_complete_semaphores =
			vector_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
		context.in_flight_fences =
			vector_reserve(VkFence, context.swapchain.max_frames_in_flight);

		for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
				VkSemaphoreCreateInfo sem_create_info = {
					VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
				vkCreateSemaphore(context.device.logical_device,
								  &sem_create_info, context.allocator,
								  &context.image_available_semaphores[i]);
				vkCreateSemaphore(context.device.logical_device,
								  &sem_create_info, context.allocator,
								  &context.queue_complete_semaphores[i]);
				vulkan_fence_create(&context, TRUE,
									&context.in_flight_fences[i]);
		}

		context.images_in_flight =
			vector_reserve(VkFence, context.swapchain.image_count);
		SF_INFO("Fences and semaphores created.");

		if (!vulkan_shader_create(&context, &context.shader)) {
				SF_ERROR("Failed to load built-in shader.");
				return FALSE;
		}

		if (!create_buffers(&context)) {
				SF_FATAL("Failed to create buffers.")
				return FALSE;
		}
		SF_INFO("Successfully created buffers.");
		SF_INFO("Vulkan renderer provider initialized successfully.");
		return TRUE;
}

void vulkan_shutdown(renderer_provider *api) {
		SF_DEBUG("Waiting for idle...");
		vkDeviceWaitIdle(context.device.logical_device);
		SF_DEBUG("Destroying sync objects.");
		for (u32 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
				if (context.image_available_semaphores[i]) {
						vkDestroySemaphore(
							context.device.logical_device,
							context.image_available_semaphores[i],
							context.allocator);
				}
				if (context.queue_complete_semaphores[i]) {
						vkDestroySemaphore(context.device.logical_device,
										   context.queue_complete_semaphores[i],
										   context.allocator);
				}
				vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
		}
		vector_destroy(context.image_available_semaphores);
		vector_destroy(context.queue_complete_semaphores);
		vector_destroy(context.in_flight_fences);
		vector_destroy(context.images_in_flight);
#if defined(DEBUG)
		SF_DEBUG("Destroying vulkan debugger.");
		if (context.debug_messenger) {
				PFN_vkDestroyDebugUtilsMessengerEXT func =
					(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
						context.instance, "vkDestroyDebugUtilsMessengerEXT");
				func(context.instance, context.debug_messenger,
					 context.allocator);
		}
#endif
		SF_DEBUG("Destroying buffers");
		vulkan_buffer_destroy(&context, &context.VBO);
		vulkan_buffer_destroy(&context, &context.IBO);
		SF_DEBUG("Destroying shader modules");
		vulkan_shader_destroy(&context, &context.shader);
		SF_DEBUG("Destroying main render pass");
		vulkan_render_pass_destroy(&context, &context.main_render_pass);
		SF_DEBUG("Destroying vulkan swapchain");
		vulkan_swapchain_destroy(&context, &context.swapchain);
		SF_DEBUG("Destroying vulkan surface");
		vkDestroySurfaceKHR(context.instance, context.surface,
							context.allocator);
		SF_DEBUG("Destroying graphics command pool.");
		for (u32 i = 0; i < context.swapchain.image_count; ++i) {
				vulkan_command_buffer_free(
					&context, context.device.graphics_command_pool,
					&context.graphics_command_buffers[i]);
		}
		vector_destroy(context.graphics_command_buffers);
		SF_DEBUG("Destroying framebuffers.");
		for (u32 i = 0; i < context.swapchain.image_count; ++i) {
				vulkan_framebuffer_destroy(&context,
										   &context.swapchain.framebuffers[i]);
		}
		vector_destroy(context.swapchain.framebuffers);
		SF_DEBUG("Destroying devices.");
		vulkan_device_destroy(&context);
		SF_DEBUG("Destroying vulkan instance.");
		vkDestroyInstance(context.instance, context.allocator);
		event_unregister(EVENT_CODE_WINDOW_RESIZED, &context, window_resized);
}

b8 vulkan_begin_frame(struct renderer_provider *api, f64 deltaTime) {
		// TODO: handle resizing from SDL side
		if (context.recreating_swapchain) {
				if (vkDeviceWaitIdle(context.device.logical_device) !=
					VK_SUCCESS) {
						SF_ERROR("Failed to wait for device idle.");
						return FALSE;
				}
				SF_INFO("Recreating swapchain.")
				return FALSE;
		}

		if (!vulkan_fence_wait(&context,
							   &context.in_flight_fences[context.current_frame],
							   UINT64_MAX)) {
				SF_WARNING("Could not wait for in-flight fence.");
				return FALSE;
		}

		if (!vulkan_swapchain_get_next_image_index(
				&context, &context.swapchain, UINT64_MAX,
				context.image_available_semaphores[context.current_frame],
				SF_NULL, &context.image_index)) {
				return FALSE;
		}

		vulkan_command_buffer *command_buffer =
			&context.graphics_command_buffers[context.image_index];
		vulkan_command_buffer_reset(command_buffer);
		vulkan_command_buffer_begin(command_buffer, FALSE, FALSE, FALSE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = (f32)context.framebuffer_height;
		viewport.width = (f32)context.framebuffer_width;
		viewport.height = -(f32)context.framebuffer_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = context.framebuffer_width;
		scissor.extent.height = context.framebuffer_height;

		vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

		context.main_render_pass.extent.w = context.framebuffer_width;
		context.main_render_pass.extent.h = context.framebuffer_height;

		vulkan_render_pass_begin(
			&context.main_render_pass, command_buffer,
			context.swapchain.framebuffers[context.image_index].handle);

		return TRUE;
}

b8 vulkan_end_frame(struct renderer_provider *api) {
		vulkan_render_pass_end(
			&context.main_render_pass,
			&context.graphics_command_buffers[context.image_index]);
		vulkan_command_buffer_end(
			&context.graphics_command_buffers[context.image_index]);
		if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) {
				vulkan_fence_wait(&context,
								  context.images_in_flight[context.image_index],
								  UINT64_MAX);
		}

		context.images_in_flight[context.image_index] =
			&context.in_flight_fences[context.current_frame];
		vulkan_fence_reset(&context,
						   &context.in_flight_fences[context.current_frame]);

		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers =
			&context.graphics_command_buffers[context.image_index].handle;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores =
			&context.queue_complete_semaphores[context.current_frame];
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores =
			&context.image_available_semaphores[context.current_frame];

		// prevents subsequent color attachment writes from executing until
		// semaphore signals
		VkPipelineStageFlags flags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.pWaitDstStageMask = flags;
		if (vkQueueSubmit(context.device.graphics_queue, 1, &submit_info,
						  context.in_flight_fences[context.current_frame]) !=
			VK_SUCCESS) {
				SF_ERROR("vkQueueSubmit failed.");
				return FALSE;
		}

		vulkan_command_buffer_update_submitted(
			&context.graphics_command_buffers[context.image_index]);

		vulkan_swapchain_present(
			&context, &context.swapchain, context.device.graphics_queue,
			context.device.present_queue,
			context.queue_complete_semaphores[context.current_frame],
			context.image_index);

		return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
				  unsigned int message_types,
				  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
				  void *user_data) {
		switch (message_severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				SF_ERROR(callback_data->pMessage);
				break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				SF_WARNING(callback_data->pMessage);
				break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				SF_INFO(callback_data->pMessage);
				break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				SF_TRACE(callback_data->pMessage);
				break;
		default:
				break;
		}
		return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
		VkPhysicalDeviceMemoryProperties mem_props;
		vkGetPhysicalDeviceMemoryProperties(context.device.physical_device,
											&mem_props);
		for (u32 i = 0; i < mem_props.memoryTypeCount; ++i) {
				if (type_filter & (1 << i) &&
					(mem_props.memoryTypes[i].propertyFlags & property_flags) ==
						property_flags) {
						return i;
				}
		}
		SF_WARNING("Failed to find suitable memory type.");
		return -1;
}

void create_command_buffers() {
		if (!context.graphics_command_buffers) {
				context.graphics_command_buffers = vector_reserve(
					vulkan_command_buffer, context.swapchain.image_count);
		}
		for (u32 i = 0; i < context.swapchain.image_count; ++i) {
				if (context.graphics_command_buffers[i].handle) {
						vulkan_command_buffer_free(
							&context, context.device.graphics_command_pool,
							&context.graphics_command_buffers[i]);
				}
				sfmemset(&context.graphics_command_buffers[i], 0,
						 sizeof(vulkan_command_buffer));
				vulkan_command_buffer_create(
					&context, context.device.graphics_command_pool, TRUE,
					&context.graphics_command_buffers[i]);
		}
		SF_INFO("Command buffers created.");
}

void recreate_frambuffers(vulkan_swapchain *swapchain,
						  vulkan_render_pass *render_pass) {
		for (u32 i = 0; i < swapchain->image_count; ++i) {
				// TODO: make this automatically adjust based on currently
				// configured attachments
				VkImageView attachments[] = {swapchain->image_views[i],
											 swapchain->depth_attachment.view};
				u32 attach_count = 2;
				vulkan_framebuffer_destroy(&context,
										   &swapchain->framebuffers[i]);
				vulkan_framebuffer_create(
					&context, render_pass, context.framebuffer_width,
					context.framebuffer_height, attach_count, attachments,
					&swapchain->framebuffers[i]);
		}
}

b8 window_resized(u16 code, void *sender, void *listener_list,
				  event_context data) {
		cached_window_width = data.data.u32[0];
		cached_window_height = data.data.u32[1];
		if (vkDeviceWaitIdle(context.device.logical_device) != VK_SUCCESS) {
				SF_ERROR("vulkan_begin_frame: failed to wait for idle.");
				return FALSE;
		}

		if (!recreate_swapchain()) {
				return FALSE;
		}
		return TRUE;
}

b8 recreate_swapchain() {
		if (context.recreating_swapchain) {
				SF_DEBUG("Already recreating the swapchain.");
				return FALSE;
		}

		if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
				SF_DEBUG("Cannot recreate swapchain with framebuffer "
						 "dimensions less than 1.");
				return FALSE;
		}

		context.recreating_swapchain = TRUE;
		vkDeviceWaitIdle(context.device.logical_device);
		for (u32 i = 0; i < context.swapchain.image_count; ++i) {
				context.images_in_flight[i] = SF_NULL;
		}
		vulkan_device_query_swapchain_support(
			context.device.physical_device, context.surface,
			&context.device.swapchain_support);
		vulkan_device_detect_depth_format(&context.device);
		vulkan_swapchain_recreate(&context, context.framebuffer_width,
								  context.framebuffer_height,
								  &context.swapchain);
		context.framebuffer_width = cached_window_width;
		context.framebuffer_height = cached_window_height;
		context.main_render_pass.extent.w = context.framebuffer_height;
		context.main_render_pass.extent.h = context.framebuffer_height;
		cached_window_width = 0;
		cached_window_height = 0;

		for (u32 i = 0; i < context.swapchain.image_count; ++i) {
				vulkan_command_buffer_free(
					&context, context.device.graphics_command_pool,
					&context.graphics_command_buffers[i]);
				vulkan_framebuffer_destroy(&context,
										   &context.swapchain.framebuffers[i]);
		}

		context.main_render_pass.extent.x = 0;
		context.main_render_pass.extent.y = 0;
		context.main_render_pass.extent.w = context.framebuffer_width;
		context.main_render_pass.extent.h = context.framebuffer_height;
		recreate_frambuffers(&context.swapchain, &context.main_render_pass);
		create_command_buffers();
		context.recreating_swapchain = FALSE;
		return TRUE;
}

//
// NOTE: temporary
void upload_data(vulkan_context *context, VkCommandPool pool, VkQueue queue,
				 vulkan_buffer *buffer, u64 size, u64 offset, void *data) {
		VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		vulkan_buffer staging;
		vulkan_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 flags, &staging);
		vulkan_buffer_bind(context, &staging, 0);

		vulkan_buffer_load_data(context, &staging, size, 0, flags, data);

		vulkan_buffer_copy(context, queue, pool, staging.handle, 0,
						   buffer->handle, offset, size);
		vulkan_buffer_destroy(context, &staging);
}

// NOTE: temporary
b8 create_buffers(vulkan_context *context) {
		VkMemoryPropertyFlagBits mem_prop_flags =
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		const u64 VBO_len = sizeof(vertex) * 1024;
		if (!vulkan_buffer_create(context, VBO_len,
								  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
									  VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								  mem_prop_flags, &context->VBO)) {
				SF_FATAL("Failed to create vertex buffer");
				return FALSE;
		}
		vulkan_buffer_bind(context, &context->VBO, 0);

		const u64 IBO_len = sizeof(u32) * 1024;
		if (!vulkan_buffer_create(context, IBO_len,
								  VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
									  VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								  mem_prop_flags, &context->IBO)) {
				SF_FATAL("Failed to create index buffer");
				return FALSE;
		}
		vulkan_buffer_bind(context, &context->IBO, 0);

		vertex verts[4];
		sfmemset(verts, 0, sizeof(vertex) * 4);

		verts[0].position.x = -0.5;
		verts[0].position.y = -0.5;

		verts[1].position.x = 0.5;
		verts[1].position.y = 0.5;

		verts[2].position.x = -0.5;
		verts[2].position.y = 0.5;

		verts[3].position.x = 0.5;
		verts[3].position.y = -0.5;

		u32 indices[6] = {0, 1, 2, 0, 3, 1};

		upload_data(context, context->device.graphics_command_pool,
					context->device.graphics_queue, &context->VBO,
					sizeof(vertex) * 4, 0, verts);

		upload_data(context, context->device.graphics_command_pool,
					context->device.graphics_queue, &context->IBO,
					sizeof(u32) * 6, 0, indices);

		return TRUE;
}

void vulkan_update_scene_data(mat4 projection, mat4 view) {
		vulkan_command_buffer *cmd_buffer =
			&context.graphics_command_buffers[context.image_index];
		vulkan_shader_bind(&context, &context.shader);
		context.scene_data.projection = projection;
		context.scene_data.view = view;
		vulkan_shader_update_uniforms(&context, &context.shader,
									  &context.scene_data);
}

void vulkan_update_objects_data(mat4 model) {
		vulkan_command_buffer *cmd_buffer =
			&context.graphics_command_buffers[context.image_index];
		vulkan_shader_update_model(&context, &context.shader, model);
		// TODO: remove this temp code
		vulkan_shader_bind(&context, &context.shader);
		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(cmd_buffer->handle, 0, 1, &context.VBO.handle,
							   (VkDeviceSize *)offsets);
		vkCmdBindIndexBuffer(cmd_buffer->handle, context.IBO.handle, 0,
							 VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd_buffer->handle, 6, 1, 0, 0, 0);
}
