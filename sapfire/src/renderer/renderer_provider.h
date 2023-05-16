#pragma once

#include "renderer_types.h"

struct platform_state;

/**
* @brief Create a renderer provider. This function is called by the platform to create a renderer provider. The platform will be passed a pointer to the platform state that is passed to the renderer_provider_create function.
* @param api The renderer API to use. Must be one of RENDERER_API_VULKAN
* @param plat_state * An optional pointer to the
* @param out_renderer_provider
*/
b8 renderer_provider_create(renderer_api api, struct platform_state* plat_state, renderer_provider* out_renderer_provider);

/**
* @brief Shuts down a renderer provider. This is called when the provider is no longer needed and should not be used anymore.
* @param provider * The renderer provider to shutdown. Must be valid
*/
void renderer_provider_shutdown(renderer_provider* provider);
