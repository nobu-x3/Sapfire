#include "containers/vector.h"
#include "core/asserts.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "core/sfstring.h"
#include "defines.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_device.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_render_pass.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "vulkan_platform.h"
#include "vulkan_provider.h"
#include "vulkan_types.h"
#include <string.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#define DEBUG
// TODO: really think about singletons...
static vulkan_context context;

i32 find_memory_index(u32 type_filter, u32 property_flags);

void recreate_frambuffers(renderer_provider *api, vulkan_swapchain *swapchain,
						  vulkan_render_pass *render_pass);

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagBitsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

void create_command_buffers(renderer_provider *api);

b8 vulkan_initialize(renderer_provider *api, const char *app_name,
					 struct platform_state *plat_state) {
		context.find_memory_index = find_memory_index;
		// TODO: config
		context.framebuffer_width = 800;
		context.framebuffer_height = 600;
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

		color color = {0.0f, 0.3f, 0.5f, 1.0f};
		extent2d extent = {0.0f, 0.0f, context.framebuffer_width,
						   context.framebuffer_height};
		vulkan_render_pass_create(&context, color, extent, 1.0f, 0,
								  &context.main_render_pass);

		context.swapchain.framebuffers =
			vector_reserve(vulkan_framebuffer, context.swapchain.image_count);
		recreate_frambuffers(api, &context.swapchain,
							 &context.main_render_pass);

		create_command_buffers(api);

		SF_INFO("Vulkan renderer provider initialized successfully.");
		return TRUE;
}

void vulkan_shutdown(renderer_provider *api) {
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
		SF_DEBUG("Destroying main render pass");
		vulkan_render_pass_destroy(&context, &context.main_render_pass);
		SF_DEBUG("Destroying vulkan swapchain");
		vulkan_swapchain_destroy(&context, &context.swapchain);
		SF_DEBUG("Destroying vulkan surface");
		vkDestroySurfaceKHR(context.instance, context.surface,
							context.allocator);
		SF_DEBUG("Waiting for idle...");
		vkQueueWaitIdle(context.device.graphics_queue);
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
}

b8 vulkan_begin_frame(struct renderer_provider *api, f64 deltaTime) {
		return TRUE;
}

b8 vulkan_end_frame(struct renderer_provider *api) { return TRUE; }

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
				  VkDebugUtilsMessageTypeFlagBitsEXT message_types,
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

void create_command_buffers(renderer_provider *api) {
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
void recreate_frambuffers(renderer_provider *api, vulkan_swapchain *swapchain,
						  vulkan_render_pass *render_pass) {
		for (u32 i = 0; i < swapchain->image_count; ++i) {
				// TODO: make this automatically adjust based on currently
				// configured attachments
				VkImageView attachments[] = {swapchain->image_views[i],
											 swapchain->depth_attachment.view};
				u32 attach_count = 2;
				vulkan_framebuffer_create(
					&context, render_pass, context.framebuffer_width,
					context.framebuffer_height, attach_count, attachments,
					&swapchain->framebuffers[i]);
		}
}
