#include "containers/vector.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "core/sfstring.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan_device.h"
#include <vulkan/vulkan_core.h>

typedef struct vulkan_physical_device_requirements {
		b8 graphics;
		b8 present;
		b8 compute;
		b8 transfer;
		const char **device_extension_names;
		b8 sampler_anisotropy;
		b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
		u32 graphics_family_index;
		u32 present_family_index;
		u32 compute_family_index;
		u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

b8 select_physical_device(vulkan_context *context);

b8 vulkan_device_create(vulkan_context *context) {
		if (!select_physical_device(context)) {
				SF_FATAL("Failed to select physical device.");
				return FALSE;
		}

		u32 index_count = 1;
		b8 present_shares_graphics = context->device.graphics_queue_index ==
									 context->device.present_queue_index;
		b8 transfer_shares_graphics = context->device.graphics_queue_index ==
									  context->device.transfer_queue_index;
		b8 transfer_shares_present = context->device.transfer_queue_index ==
									 context->device.present_queue_index;
		if (!present_shares_graphics) {
				index_count += 1;
		}
		if (!transfer_shares_graphics) {
				index_count += 1;
		}
		u32 indices[32];
		u8 index = 0;
		indices[index] = context->device.graphics_queue_index;
		if (!present_shares_graphics) {
				indices[index += 1] = context->device.present_queue_index;
		}
		if (!transfer_shares_graphics) {
				indices[index += 1] = context->device.transfer_queue_index;
		}
		VkDeviceQueueCreateInfo q_create_infos[32];
		for (u32 i = 0; i < index_count; ++i) {
				q_create_infos[i].sType =
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				q_create_infos[i].pNext = SF_NULL;
				q_create_infos[i].queueCount = 1;
				q_create_infos[i].queueFamilyIndex = indices[i];
				f32 q_prio = 1.0f;
				q_create_infos[i].pQueuePriorities = &q_prio;
				q_create_infos[i].flags = 0;
		}

		// TODO: make this configurable
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
		VkPhysicalDeviceFeatures device_features;
#else

		VkPhysicalDeviceFeatures device_features = {};
#endif
		device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo device_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
		device_create_info.queueCreateInfoCount = index_count;
		device_create_info.pQueueCreateInfos = q_create_infos;
		const char *ext_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		device_create_info.enabledExtensionCount = 1;
		device_create_info.ppEnabledExtensionNames = &ext_names;
		device_create_info.pEnabledFeatures = &device_features;

		VK_ASSERT_SUCCESS(
			vkCreateDevice(context->device.physical_device, &device_create_info,
						   context->allocator, &context->device.logical_device),
			"Failed to create logical device.");
		SF_INFO("Logical device created.");

		vkGetDeviceQueue(context->device.logical_device,
						 context->device.graphics_queue_index, 0,
						 &context->device.graphics_queue);
		vkGetDeviceQueue(context->device.logical_device,
						 context->device.present_queue_index, 0,
						 &context->device.present_queue);
		vkGetDeviceQueue(context->device.logical_device,
						 context->device.transfer_queue_index, 0,
						 &context->device.transfer_queue);
		SF_INFO("Queues obtained.");

		VkCommandPoolCreateInfo pool_create_info = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
		pool_create_info.queueFamilyIndex =
			context->device.graphics_queue_index;
		pool_create_info.flags =
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_ASSERT_SUCCESS(
			vkCreateCommandPool(context->device.logical_device,
								&pool_create_info, context->allocator,
								&context->device.graphics_command_pool),
			"Failed to create graphics command pool");
		SF_INFO("Graphics command pool created.");

		return TRUE;
}

void vulkan_device_destroy(vulkan_context *context) {

		context->device.graphics_queue = SF_NULL;
		context->device.present_queue = SF_NULL;
		context->device.transfer_queue = SF_NULL;
		vkDestroyCommandPool(context->device.logical_device,
							 context->device.graphics_command_pool,
							 context->allocator);
		context->device.graphics_command_pool = SF_NULL;
		if (context->device.logical_device) {
				vkDestroyDevice(context->device.logical_device,
								context->allocator);
				context->device.logical_device = SF_NULL;
		}
		if (context->device.swapchain_support.formats) {
				sffree(context->device.swapchain_support.formats,
					   sizeof(VkSurfaceFormatKHR) *
						   context->device.swapchain_support.format_count,
					   MEMORY_TAG_RENDERER);
				context->device.swapchain_support.formats = SF_NULL;
				context->device.swapchain_support.format_count = 0;
		}
		if (context->device.swapchain_support.present_modes) {
				sffree(context->device.swapchain_support.present_modes,
					   sizeof(VkPresentModeKHR) *
						   context->device.swapchain_support.present_mode_count,
					   MEMORY_TAG_RENDERER);
				context->device.swapchain_support.present_modes = SF_NULL;
				context->device.swapchain_support.present_mode_count = 0;
		}

		context->device.physical_device = SF_NULL;
		sfmemset(&context->device.swapchain_support.capabilities, 0,
				 sizeof(context->device.swapchain_support.capabilities));
		context->device.graphics_queue_index = -1;
		context->device.present_queue_index = -1;
		context->device.transfer_queue_index = -1;
		// NOTE: also compute if enabled
}

b8 vulkan_device_detect_depth_format(vulkan_device *device) {
		const u32 count = 3;
		VkFormat priorities[3] = {
			VK_FORMAT_D32_SFLOAT, // Don't really care about stencil, so
								  // this is top 1 prio
			VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
		u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		for (u32 i = 0; i < count; ++i) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(device->physical_device,
													priorities[i], &props);
				if ((props.linearTilingFeatures & flags) == flags) {
						device->depth_format = priorities[i];
						return TRUE;
				} else if ((props.optimalTilingFeatures & flags) == flags) {
						device->depth_format = priorities[i];
						return TRUE;
				}
		}
		return FALSE;
}

b8 physical_device_meets_requirements(
	VkPhysicalDevice device, VkSurfaceKHR surface,
	const VkPhysicalDeviceProperties *properties,
	const VkPhysicalDeviceFeatures *features,
	const vulkan_physical_device_requirements *requirements,
	vulkan_physical_device_queue_family_info *out_queue_family_info,
	vulkan_swapchain_support_info *out_swapchain_support);

// NOTE: this stuff is copied from Kohi
b8 select_physical_device(vulkan_context *context) {
		u32 phys_device_count = 0;
		VK_ASSERT_SUCCESS(vkEnumeratePhysicalDevices(
							  context->instance, &phys_device_count, SF_NULL),
						  "Failed to enumerate physical devices.");
		if (phys_device_count <= 0) {
				SF_FATAL("No vulkan-capable physical devices found.");
				return FALSE;
		}
		VkPhysicalDevice phys_devices[32];
		VK_ASSERT_SUCCESS(vkEnumeratePhysicalDevices(context->instance,
													 &phys_device_count,
													 phys_devices),
						  "Failed to enumerate physical devices.");

		for (u32 i = 0; i < phys_device_count; ++i) {
				VkPhysicalDeviceProperties properties;
				vkGetPhysicalDeviceProperties(phys_devices[i], &properties);

				VkPhysicalDeviceFeatures features;
				vkGetPhysicalDeviceFeatures(phys_devices[i], &features);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
				VkPhysicalDeviceMemoryProperties memory;
#else
				VkPhysicalDeviceMemoryProperties memory = {};
#endif
				vkGetPhysicalDeviceMemoryProperties(phys_devices[i], &memory);

				// TODO: make this configurable by the engine
				vulkan_physical_device_requirements requirements;
				requirements.graphics = TRUE;
				requirements.present = TRUE;
				requirements.transfer = TRUE;
				// NOTE: Enable this if compute will be required.
				// requirements.compute = TRUE;
				requirements.sampler_anisotropy = TRUE;
				requirements.discrete_gpu = TRUE;
				requirements.device_extension_names =
					vector_create(const char *);
				vector_push(requirements.device_extension_names,
							&VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
				vulkan_physical_device_queue_family_info queue_info;
#else
				vulkan_physical_device_queue_family_info queue_info = {};
#endif
				b8 result = physical_device_meets_requirements(
					phys_devices[i], context->surface, &properties, &features,
					&requirements, &queue_info,
					&context->device.swapchain_support);
				vector_destroy(requirements.device_extension_names);
				if (result) {
						SF_INFO("Selected device: '%s'.",
								properties.deviceName);
						// GPU type, etc.
						switch (properties.deviceType) {
						default:
						case VK_PHYSICAL_DEVICE_TYPE_OTHER:
								SF_INFO("GPU type is Unknown.");
								break;
						case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
								SF_INFO("GPU type is Integrated.");
								break;
						case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
								SF_INFO("GPU type is Descrete.");
								break;
						case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
								SF_INFO("GPU type is Virtual.");
								break;
						case VK_PHYSICAL_DEVICE_TYPE_CPU:
								SF_INFO("GPU type is CPU.");
								break;
						}

						SF_INFO("GPU Driver version: %d.%d.%d",
								VK_VERSION_MAJOR(properties.driverVersion),
								VK_VERSION_MINOR(properties.driverVersion),
								VK_VERSION_PATCH(properties.driverVersion));

						// Vulkan API version.
						SF_INFO("Vulkan API version: %d.%d.%d",
								VK_VERSION_MAJOR(properties.apiVersion),
								VK_VERSION_MINOR(properties.apiVersion),
								VK_VERSION_PATCH(properties.apiVersion));

						// Memory information
						for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
								f32 memory_size_gib =
									(((f32)memory.memoryHeaps[j].size) /
									 1024.0f / 1024.0f / 1024.0f);
								if (memory.memoryHeaps[j].flags &
									VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
										SF_INFO("Local GPU memory: %.2f GiB",
												memory_size_gib);
								} else {
										SF_INFO(
											"Shared System memory: %.2f GiB",
											memory_size_gib);
								}
						}

						context->device.physical_device = phys_devices[i];
						context->device.graphics_queue_index =
							queue_info.graphics_family_index;
						context->device.present_queue_index =
							queue_info.present_family_index;
						context->device.transfer_queue_index =
							queue_info.transfer_family_index;
						// NOTE: set compute index here if needed.

						// Keep a copy of properties, features and memory info
						// for later use.
						context->device.properties = properties;
						context->device.features = features;
						context->device.memory = memory;
						break;
				}
		}

		// Ensure a device was selected
		if (!context->device.physical_device) {
				SF_FATAL("No physical devices were found which meet the "
						 "requirements.");
				return FALSE;
		}

		SF_INFO("Physical device selected.");
		return TRUE;
}

void vulkan_device_query_swapchain_support(
	VkPhysicalDevice physical_device, VkSurfaceKHR surface,
	vulkan_swapchain_support_info *out_support_info) {
		// Surface capabilities
		VK_ASSERT_SUCCESS(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physical_device, surface, &out_support_info->capabilities),
			"Failed to query physical device surface capabilities.");

		// Surface formats
		VK_ASSERT_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
							  physical_device, surface,
							  &out_support_info->format_count, SF_NULL),
						  "Failed to query physical device surface formats.");

		if (out_support_info->format_count != 0) {
				if (!out_support_info->formats) {
						out_support_info->formats =
							sfalloc(sizeof(VkSurfaceFormatKHR) *
										out_support_info->format_count,
									MEMORY_TAG_RENDERER);
				}
				VK_ASSERT_SUCCESS(
					vkGetPhysicalDeviceSurfaceFormatsKHR(
						physical_device, surface,
						&out_support_info->format_count,
						out_support_info->formats),
					"Fauled to query physical device surface formats.");
		}

		// Present modes
		VK_ASSERT_SUCCESS(
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				physical_device, surface, &out_support_info->present_mode_count,
				SF_NULL),
			"Failed to query physical device surface present modes.");
		if (out_support_info->present_mode_count != 0) {
				if (!out_support_info->present_modes) {
						out_support_info->present_modes =
							sfalloc(sizeof(VkPresentModeKHR) *
										out_support_info->present_mode_count,
									MEMORY_TAG_RENDERER);
				}
				VK_ASSERT_SUCCESS(
					vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device, surface,
						&out_support_info->present_mode_count,
						out_support_info->present_modes),
					"Failed to query physical device surface present modes.");
		}
}

b8 physical_device_meets_requirements(
	VkPhysicalDevice device, VkSurfaceKHR surface,
	const VkPhysicalDeviceProperties *properties,
	const VkPhysicalDeviceFeatures *features,
	const vulkan_physical_device_requirements *requirements,
	vulkan_physical_device_queue_family_info *out_queue_info,
	vulkan_swapchain_support_info *out_swapchain_support) {
		out_queue_info->graphics_family_index = -1;
		out_queue_info->present_family_index = -1;
		out_queue_info->compute_family_index = -1;
		out_queue_info->transfer_family_index = -1;
		if (requirements->discrete_gpu) {
				if (properties->deviceType !=
					VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
						SF_INFO("Device is not a discrete GPU, and one is "
								"required. Skipping.");
						return FALSE;
				}
		}

		u32 queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
												 0);
		VkQueueFamilyProperties queue_families[32];
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
												 queue_families);

		// Look at each queue and see what queues it supports
		SF_INFO("Graphics | Present | Compute | Transfer | Name");
		u8 min_transfer_score = 255;
		for (u32 i = 0; i < queue_family_count; ++i) {
				u8 current_transfer_score = 0;

				// Graphics queue?
				if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						out_queue_info->graphics_family_index = i;
						++current_transfer_score;
				}

				// Compute queue?
				if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
						out_queue_info->compute_family_index = i;
						++current_transfer_score;
				}

				// Transfer queue?
				if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
						// Take the index if it is the current lowest. This
						// increases the liklihood that it is a dedicated
						// transfer queue.
						if (current_transfer_score <= min_transfer_score) {
								min_transfer_score = current_transfer_score;
								out_queue_info->transfer_family_index = i;
						}
				}

				// Present queue?
				VkBool32 supports_present = VK_FALSE;
				VK_ASSERT_SUCCESS(
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
														 &supports_present),
					"Failed to query physical device surface support.");
				if (supports_present) {
						out_queue_info->present_family_index = i;
				}
		}

		// Print out some info about the device
		SF_INFO("       %d |       %d |       %d |        %d | %s",
				out_queue_info->graphics_family_index != -1,
				out_queue_info->present_family_index != -1,
				out_queue_info->compute_family_index != -1,
				out_queue_info->transfer_family_index != -1,
				properties->deviceName);

		if ((!requirements->graphics ||
			 (requirements->graphics &&
			  out_queue_info->graphics_family_index != -1)) &&
			(!requirements->present ||
			 (requirements->present &&
			  out_queue_info->present_family_index != -1)) &&
			(!requirements->compute ||
			 (requirements->compute &&
			  out_queue_info->compute_family_index != -1)) &&
			(!requirements->transfer ||
			 (requirements->transfer &&
			  out_queue_info->transfer_family_index != -1))) {
				SF_INFO("Device meets queue requirements.");
				SF_TRACE("Graphics Family Index: %i",
						 out_queue_info->graphics_family_index);
				SF_TRACE("Present Family Index:  %i",
						 out_queue_info->present_family_index);
				SF_TRACE("Transfer Family Index: %i",
						 out_queue_info->transfer_family_index);
				SF_TRACE("Compute Family Index:  %i",
						 out_queue_info->compute_family_index);

				// Query swapchain support.
				vulkan_device_query_swapchain_support(device, surface,
													  out_swapchain_support);

				if (out_swapchain_support->format_count < 1 ||
					out_swapchain_support->present_mode_count < 1) {
						if (out_swapchain_support->formats) {
								sffree(out_swapchain_support->formats,
									   sizeof(VkSurfaceFormatKHR) *
										   out_swapchain_support->format_count,
									   MEMORY_TAG_RENDERER);
						}
						if (out_swapchain_support->present_modes) {
								sffree(out_swapchain_support->present_modes,
									   sizeof(VkPresentModeKHR) *
										   out_swapchain_support
											   ->present_mode_count,
									   MEMORY_TAG_RENDERER);
						}
						SF_INFO("Required swapchain support not present, "
								"skipping device.");
						return FALSE;
				}

				// Device extensions.
				if (requirements->device_extension_names) {
						u32 available_extension_count = 0;
						VkExtensionProperties *available_extensions = 0;
						VK_ASSERT_SUCCESS(
							vkEnumerateDeviceExtensionProperties(
								device, 0, &available_extension_count, 0),
							"Failed to enumerate device extension properties.");
						if (available_extension_count != 0) {
								available_extensions =
									sfalloc(sizeof(VkExtensionProperties) *
												available_extension_count,
											MEMORY_TAG_RENDERER);
								VK_ASSERT_SUCCESS(
									vkEnumerateDeviceExtensionProperties(
										device, 0, &available_extension_count,
										available_extensions),
									"Failed to enumerate device extension "
									"properties.");

								u32 required_extension_count = vector_len(
									requirements->device_extension_names);
								for (u32 i = 0; i < required_extension_count;
									 ++i) {
										b8 found = FALSE;
										for (u32 j = 0;
											 j < available_extension_count;
											 ++j) {
												if (sfstreq(
														requirements
															->device_extension_names
																[i],
														available_extensions[j]
															.extensionName)) {
														found = TRUE;
														break;
												}
										}

										if (!found) {
												SF_INFO(
													"Required extension not "
													"found: '%s', skipping "
													"device.",
													requirements
														->device_extension_names
															[i]);
												sffree(
													available_extensions,
													sizeof(
														VkExtensionProperties) *
														available_extension_count,
													MEMORY_TAG_RENDERER);
												return FALSE;
										}
								}
						}
						sffree(available_extensions,
							   sizeof(VkExtensionProperties) *
								   available_extension_count,
							   MEMORY_TAG_RENDERER);
				}

				// Sampler anisotropy
				if (requirements->sampler_anisotropy &&
					!features->samplerAnisotropy) {
						SF_INFO("Device does not support samplerAnisotropy, "
								"skipping.");
						return FALSE;
				}

				// Device meets all requirements.
				return TRUE;
		}

		return FALSE;
}
