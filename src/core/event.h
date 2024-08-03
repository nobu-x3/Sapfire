#pragma once

#include "defines.h"

typedef struct event_context {
	union { // 128 bytes
		i64 i64[2];
		u64 u64[2];
		f64 f64[2];

		i32 i32[4];
		u32 u32[4];
		f32 f32[4];

		i16 i16[8];
		u16 u16[8];

		char c[16];
	} data;
} event_context;

typedef b8 (*PFN_on_event) (u16 code, void *sender, void *listener_list,
							event_context data);

/**
 * @brief Initializes the event system. If memory is NULL, will populate mem_size.
 *
 *	@param mem_size Holds the required memory size of the internal state.
 * @param memory NULL if requesting memory size, otherwise allocated block of
 *memory.
 * @return TRUE on success; otherwise FALSE.
 */
b8 event_initialize (u64 *mem_size, void *memory);

/**
* @brief Shuts down the event system. This is called at shutdown time to clean up memory allocated for the event system
* @param memory Pointer to the memory
*/
void event_shutdown (void *memory);

/**
* @brief Register an event handler. Registers a callback to be called when an event occurs. This is useful for event handling code to determine which event handler to call based on the event code.
* @param code The event code that will be used to identify the event.
* @param listener Pointer to the listener.
* @param on_event The function pointer that will be called when the event occurs.
* @return TRUE if the event was registered FALSE if it was already registered or if an error occurred during registration. Note that you can't register a listener twice
*/
SAPI b8 event_register (u16 code, void *listener, PFN_on_event on_event);

/**
* @brief Unregister an event with the event subsystem. This function unregisters a previously registered event with the event subsystem.
* @param code The event code to unregister. This is used to identify the event in the event subsystem.
* @param listener The pointer to the event listener.
* @param on_event The callback function that will be called when the event occurs.
* @return TRUE if the event was unregistered FALSE otherwise. Note that you can't unregister an event that wasn't registered
*/
SAPI b8 event_unregister (u16 code, void *listener, PFN_on_event on_event);

/**
* @brief Fires an event. This is called by the event subsystem to notify all registered events that a particular event has occurred.
* @param code The event code of the event to fire.
* @param sender The sender of the event. This can be NULL if there is no sender.
* @param context The context to pass to the callback function.
* @return TRUE if an event was fired FALSE otherwise. Note that the event may be consumed by another event subsystem
*/
SAPI b8 event_fire (u16 code, void *sender, event_context context);

// Internal event codes. Application codes should use codes beyond 255.
typedef enum event_code {
	EVENT_CODE_APPLICATION_QUIT = 0x01,

	// u16[0] - key code from input.h
	EVENT_CODE_KEY_PRESSED,
	EVENT_CODE_KEY_RELEASED,

	// u16[0] - button key from input.h
	EVENT_CODE_MOUSE_BUTTON_PRESSED,
	EVENT_CODE_MOUSE_BUTTON_RELEASED,

	// u32[0] - new X, u32[1] - new Y
	EVENT_CODE_MOUSE_MOVED,
	// u32[2] - delta Z
	EVENT_CODE_MOUSE_WHEEL,

	// u32[0] - width, u32[1] - height
	EVENT_CODE_WINDOW_RESIZED,

	MAX_EVENT_CODE = 0xFF
} event_code;
