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
} input_state;

static b8 is_initialized = FALSE;
static input_state state;

b8 input_initialize() {
		if (is_initialized) {
				SF_ERROR("Attempted to initialize the input subsystem when "
						 "it's already initialized.");
				return FALSE;
		}
		sfmemset(&state, 0, sizeof(state));
		is_initialized = TRUE;
		return TRUE;
}
void input_shutdown() { is_initialized = FALSE; }
void input_update(f64 deltaTime) {
		if (!is_initialized) {
				SF_ERROR("Input update called when the input subsystem was not "
						 "initialized.");
				return;
		}

		sfmemcpy(&state.keyboard_last, &state.keyboard_current,
				 sizeof(keyboard_state));
		sfmemcpy(&state.mouse_last, &state.mouse_current, sizeof(mouse_state));
}

// keyboard
b8 input_is_key_down(keys key) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.keyboard_current.keys[key] == TRUE;
}

b8 input_is_key_up(keys key) {

		if (!is_initialized) {
				return FALSE;
		}
		return state.keyboard_current.keys[key] == FALSE;
}

b8 input_was_key_down(keys key) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.keyboard_last.keys[key] == TRUE;
}

b8 input_was_key_up(keys key) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.keyboard_last.keys[key] == FALSE;
}

void input_process_key(keys key, b8 pressed) {
		if (state.keyboard_current.keys[key] != pressed) {
				state.keyboard_current.keys[key] = pressed;
				event_context context;
				context.data.u16[0] = key;
				event_fire(pressed ? EVENT_CODE_KEY_PRESSED
								   : EVENT_CODE_KEY_RELEASED,
						   (void *)0, context);
		}
}

// mouse
b8 input_is_mouse_button_down(mouse_button button) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.mouse_current.buttons[button] == TRUE;
}

b8 input_is_mouse_button_up(mouse_button button) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.mouse_current.buttons[button] == FALSE;
}

b8 input_was_mouse_button_up(mouse_button button) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.mouse_last.buttons[button] == TRUE;
}

b8 input_was_mouse_button_down(mouse_button button) {
		if (!is_initialized) {
				return FALSE;
		}
		return state.mouse_last.buttons[button] == FALSE;
}

void input_get_mouse_position(i32 *x, i32 *y) {
		if (!is_initialized) {
				*x = 0;
				*y = 0;
				return;
		}
		*x = state.mouse_current.x;
		*y = state.mouse_current.y;
}

void input_get_last_mouse_position(i32 *x, i32 *y) {
		if (!is_initialized) {
				*x = 0;
				*y = 0;
				return;
		}
		*x = state.mouse_last.x;
		*y = state.mouse_last.y;
}

void input_process_mouse_button(mouse_button button, b8 pressed) {
		if (state.mouse_current.buttons[button] != pressed) {
				state.mouse_current.buttons[button] = pressed;
				event_context context;
				context.data.u16[0] = button;
				event_fire(pressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED
								   : EVENT_CODE_MOUSE_BUTTON_PRESSED,
						   (void *)0, context);
		}
}

void input_process_mouse_move(i32 x, i32 y) {
		if (state.mouse_current.x != x || state.mouse_current.y != y) {
				// SF_DEBUG("Mouse X: %d, Mouse Y: %d", x, y);
				state.mouse_current.x = x;
				state.mouse_current.y = y;
				event_context context;
				context.data.u32[0] = x;
				context.data.u32[1] = y;
				event_fire(EVENT_CODE_MOUSE_MOVED, SF_NULL, context);
		}
}

void input_process_mouse_wheel(i32 z) {
		event_context context;
		context.data.u32[2] = z;
		event_fire(EVENT_CODE_MOUSE_WHEEL, SF_NULL, context);
}
