#pragma once

#include "core/logger.h"
#include "defines.h"

typedef struct platform_state {
	void *internal_state;
} platform_state;

/**
* @brief Initializes the SoundFont platform. This is called by the platform_init function to initialize the SDL state and create a window.
* @param plat_state * Pointer to the platform state
* @param app_name Name of the application to display
* @param x X position in screen coordinates of the window to display
* @param y Y position in screen coordinates of the window to display
* @param width Width of the window to display in screen coordinates
* @param height Height of the window to display in screen coordinates
* @param render_api Render API to use for rendering.
* @return TRUE on success FALSE on failure ( in which case SF_FATAL is called ). Note that SDL_CreateWindow does not return a value
*/
b8 platform_init (platform_state *plat_state, const char *app_name, i32 x,
				  i32 y, i32 width, i32 height, u8 render_api);

/**
* @brief Shut down the platform. This is called at shutdown time to clean up resources that were allocated by platform_init
* @param plat_state * Platform state
*/
void platform_shutdown (platform_state *plat_state);

/**
* @brief Update the state of the platform. This is called at the end of each frame to update the internal state and any events that need to be handled.
* @param plat_state * A pointer to the platform state that is to be updated.
* @return TRUE if the platform should quit FALSE otherwise. Note that it is safe to call this function multiple times
*/
b8 platform_update_internal_state (platform_state *plat_state);

/**
* @brief Allocate memory. This is called by malloc to allocate memory for use as a data store. The caller must ensure that there is enough space in the data store to accommodate the requested size.
* @param size The size of the memory to allocate in bytes.
* @param aligned A flag indicating whether or not to align the memory
*/
void *platform_allocate (u64 size, b8 aligned);

/**
* @brief Free memory allocated by platform_alloc. This is a wrapper around free that does not check for alignment.
* @param block Pointer to the block to free. It must be aligned to 8 bytes.
* @param aligned If non - zero align the block to 8
*/
void platform_free (void *block, b8 aligned);

/**
* @brief Set memory to a value. This is a wrapper around memset that does not check for overflow.
* @param dest The address to write to. Must be aligned on 4K boundary.
* @param value The value to write to the memory. If value is - 1 it will be treated as zero.
* @param size The size of the memory to write in bytes
*/
void *platform_set_memory (void *dest, i32 value, u64 size);

/**
* @brief Copy memory from one location to another. This is a wrapper around memcpy that doesn't check for overflow.
* @param dest The address to copy to.
* @param source The address to copy from.
* @param size The size of the memory to copy in bytes
*/
void *platform_copy_memory (void *dest, const void *source, u64 size);

/**
* @brief Write a message to the console. This is a wrapper around SDL_LogInfo / SDL_LogWarn / platform_console_write_error
* @param message * The message to write.
* @param level The level of the message to write ( LOG_LEVEL_DEBUG LOG_LEVEL_INFO LOG_LEVEL_TRACE
*/
void platform_console_write (const char *message, log_level level);

/**
* @brief Writes an error message to the console. This is a wrapper around SDL_LogCritical or SDL_LogError
* @param message The message to write.
* @param fatal Whether or not the message is fatal ( true ) or error ( false )
*/
void platform_console_write_error (const char *message, b8 fatal);

/**
* @brief Get the extent of the drawable window. This is used to determine how much space is available to draw the framebuffer
* @param plat_state * A platform state to use
* @return An extent2d containing the width and height of the framebuffer in pixel units. The extent will be empty if there is no drawable
*/
extent2d platform_get_drawable_extent (platform_state *state);

/**
* @brief Get the number of milliseconds since boot. This is useful for debugging purposes. It's a 64 - bit value so we don't have to worry about overflowing.
* @return the number of milliseconds since boot
*/
u64 platform_get_absolute_time ();

f64 platform_get_delta_time ();

/**
* @brief Sleep for a number of milliseconds. This is a wrapper for SDL_Delay (). The difference between this function and platform_sleep () is that it doesn't take into account the delay in the case of an interrupt.
* @param ms The number of milliseconds to sleep for. A value of 0 means to sleep indefinitely
*/
void platform_sleep (u32 ms);