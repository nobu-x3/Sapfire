#include "core/event.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "input.h"
#include <string.h>

typedef struct keyboard_state {
	b8 keys[KEYS_MAX];
} keyboard_state;

typedef struct mouse_state {
	i32 x, y;
	u8 buttons[MB_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
	keyboard_state keyboard_current, keyboard_last;
	mouse_state mouse_current, mouse_last;
	b8 initialized;
} input_state;

static input_state *pState;

b8 input_initialize (u64 *mem_size, void *mem_block) {
	*mem_size = sizeof (input_state);
	if (mem_block == SF_NULL) { return FALSE; }
	pState				= mem_block;
	pState->initialized = TRUE;
	SF_INFO ("Input subsystem initialized successfully.");
	return TRUE;
}
void input_shutdown (void *mem_block) { pState = SF_NULL; }
void input_update (f64 deltaTime) {
	if (!pState->initialized) {
		SF_ERROR (
			"Input update called when the input subsystem was not "
			"initialized.");
		return;
	}

	sfmemcpy (&pState->keyboard_last, &pState->keyboard_current,
			  sizeof (keyboard_state));
	sfmemcpy (&pState->mouse_last, &pState->mouse_current,
			  sizeof (mouse_state));
}

// keyboard
b8 input_is_key_down (keys key) {
	if (!pState->initialized) { return FALSE; }
	return pState->keyboard_current.keys[key] == TRUE;
}

b8 input_is_key_up (keys key) {

	if (!pState->initialized) { return FALSE; }
	return pState->keyboard_current.keys[key] == FALSE;
}

b8 input_was_key_down (keys key) {
	if (!pState->initialized) { return FALSE; }
	return pState->keyboard_last.keys[key] == TRUE;
}

b8 input_was_key_up (keys key) {
	if (!pState->initialized) { return FALSE; }
	return pState->keyboard_last.keys[key] == FALSE;
}

void input_process_key (keys key, b8 pressed) {
	if (pState->keyboard_current.keys[key] != pressed) {
		pState->keyboard_current.keys[key] = pressed;
		event_context context;
		context.data.u16[0] = key;
		event_fire (pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED,
					(void *)0, context);
	}
}

// mouse
b8 input_is_mouse_button_down (mouse_button button) {
	if (!pState->initialized) { return FALSE; }
	return pState->mouse_current.buttons[button] == TRUE;
}

b8 input_is_mouse_button_up (mouse_button button) {
	if (!pState->initialized) { return FALSE; }
	return pState->mouse_current.buttons[button] == FALSE;
}

b8 input_was_mouse_button_up (mouse_button button) {
	if (!pState->initialized) { return FALSE; }
	return pState->mouse_last.buttons[button] == TRUE;
}

b8 input_was_mouse_button_down (mouse_button button) {
	if (!pState->initialized) { return FALSE; }
	return pState->mouse_last.buttons[button] == FALSE;
}

void input_get_mouse_position (i32 *x, i32 *y) {
	if (!pState->initialized) {
		*x = 0;
		*y = 0;
		return;
	}
	*x = pState->mouse_current.x;
	*y = pState->mouse_current.y;
}

void input_get_last_mouse_position (i32 *x, i32 *y) {
	if (!pState->initialized) {
		*x = 0;
		*y = 0;
		return;
	}
	*x = pState->mouse_last.x;
	*y = pState->mouse_last.y;
}

void input_process_mouse_button (mouse_button button, b8 pressed) {
	if (pState->mouse_current.buttons[button] != pressed) {
		pState->mouse_current.buttons[button] = pressed;
		event_context context;
		context.data.u16[0] = button;
		event_fire (pressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED
							: EVENT_CODE_MOUSE_BUTTON_PRESSED,
					(void *)0, context);
	}
}

void input_process_mouse_move (i32 x, i32 y) {
	if (pState->mouse_current.x != x || pState->mouse_current.y != y) {
		// SF_DEBUG("Mouse X: %d, Mouse Y: %d", x, y);
		pState->mouse_current.x = x;
		pState->mouse_current.y = y;
		event_context context;
		context.data.u32[0] = x;
		context.data.u32[1] = y;
		event_fire (EVENT_CODE_MOUSE_MOVED, SF_NULL, context);
	}
}

void input_process_mouse_wheel (i32 z) {
	event_context context;
	context.data.u32[2] = z;
	event_fire (EVENT_CODE_MOUSE_WHEEL, SF_NULL, context);
}
