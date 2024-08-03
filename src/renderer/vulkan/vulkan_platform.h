#pragma once

#include "defines.h"

struct platform_state;
struct vulkan_context;

/**
* @brief Create a Vulkan surface. This is a low - level function that should be called by the user to create a Vulkan surface.
* @param plat_state * A platform state structure previously allocated with platform_create_vulkan_context ().
* @param context * A pointer to the videobuffer context.
* @return 0 on success non - zero on failure. Errors are : SDL_E_INVALID_CONTEXT if the context is invalid
*/
b8 platform_create_vulkan_surface (struct platform_state* plat_state,
								   struct vulkan_context* context);

/**
* @brief Get a vector of the names of Vulkan instance extensions that are required to be loaded. This is useful for ensuring that a platform - specific set of extensions is available before calling platform_load_vulkan_extension ()
* @param plat_state * A pointer to the platform state
* @param names_vec A pointer to the vector to fill
*/
void platform_get_required_extension_names (struct platform_state* plat_state,
											const char*** names_vec);
