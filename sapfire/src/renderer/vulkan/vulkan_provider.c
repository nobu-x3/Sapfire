#include "core/logger.h"
#include "vulkan_provider.h"
#include "vulkan_types.h"
#include <vulkan/vulkan_core.h>

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

		VkInstanceCreateInfo create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = 0;
		create_info.ppEnabledExtensionNames = SF_NULL;
		create_info.enabledLayerCount = 0;
		create_info.ppEnabledLayerNames = SF_NULL;

		VkResult result = vkCreateInstance(&create_info, context.allocator,
										   &context.instance);
		if (result != VK_SUCCESS) {
				SF_FATAL("Failed to create VkInstance.");
				return FALSE;
		}
		return TRUE;
}

void vulkan_shutdown(renderer_provider *api) {}

b8 vulkan_begin_frame(struct renderer_provider *api, f64 deltaTime) {
		return TRUE;
}

b8 vulkan_end_frame(struct renderer_provider *api) { return TRUE; }
