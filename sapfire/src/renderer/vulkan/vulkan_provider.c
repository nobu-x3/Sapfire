#include "containers/vector.h"
#include "core/logger.h"
#include "vulkan_platform.h"
#include "vulkan_provider.h"
#include "vulkan_types.h"
#include <string.h>
#include <vulkan/vulkan_core.h>
#define DEBUG
// TODO: really think about singletons...
static vulkan_context context;

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
		vector_clear(valid_names);
		vector_clear(ext_names);
		return TRUE;
}

void vulkan_shutdown(renderer_provider *api) {}

b8 vulkan_begin_frame(struct renderer_provider *api, f64 deltaTime) {
		return TRUE;
}

b8 vulkan_end_frame(struct renderer_provider *api) { return TRUE; }
