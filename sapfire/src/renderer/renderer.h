#pragma once

#include "renderer_types.h"

/**
* @brief Initialize a renderer. This is the entry point for renderer_create () and renderer_initialize ().
* @param renderer The renderer to initialize. Mustn't be NULL.
* @param api The API to use for the renderer. Mustn't be NULL.
* @param application_name The application name to use for the renderer. May be NULL in which case the renderer will default to the application name specified in the platform_
* @param plat_state
*/
b8 renderer_initialize (renderer *renderer, renderer_api api,
						const char *application_name,
						struct platform_state *plat_state);

/**
* @brief Shut down a renderer. This is called when the renderer is no longer needed but should be called at some point in the meantime to free the memory allocated by renderer_init
* @param renderer The renderer to shutdown
*/
void renderer_shutdown (renderer *renderer);

/**
* @brief Draw a frame. This is the entry point for rendering and frame data. The renderer must be registered with the renderer_register_render () function before calling this function.
* @param renderer The renderer to draw to. This is a pointer to the renderer that is currently being rendered.
* @param bundle An optional bundle of data to be used for rendering.
* @return TRUE on success FALSE on failure. If FALSE is returned you should check the return value to see what went wrong
*/
b8 renderer_draw_frame (renderer *renderer, render_bundle *bundle);
void renderer_create_texture(renderer* renderer, const char* name, u32 width, u32 height, u32 channels, b8 opaque, const u8* pixels, struct texture* out_texture);
void renderer_destroy_texture(renderer* renderer, struct texture* texture);
