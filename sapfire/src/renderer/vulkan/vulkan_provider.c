#include "containers/vector.h"
#include "core/asserts.h"
#include "core/logger.h"
#include "vulkan_platform.h"
#include "vulkan_provider.h"
#include "vulkan_types.h"
#include <string.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#define DEBUG
// TODO: really think about singletons...
static vulkan_context context;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagBitsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

b8 vulkan_initialize(renderer_provider *api, const char *app_name,
					 struct platform_state *plat_state) {
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
			vkEnumerateInstanceLayerProperties(&available_count, 0),
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
						if (strcmp(valid_names[i],
								   available_properties[i].layerName)) {
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
