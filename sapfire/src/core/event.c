#include "containers/vector.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "event.h"

// Should be enough, if not expand
#define MAX_EVENT_CODES 4096

typedef struct registered_event {
		void *listener;
		PFN_on_event callback;
} registered_event;

typedef struct event_code_entry {
		registered_event *events;
} event_code_entry;

typedef struct event_system_state {
		event_code_entry registered[MAX_EVENT_CODES];
} event_system_state;

static b8 is_initialized = FALSE;
static event_system_state state;

b8 event_initialize() {
		if (is_initialized) {
				SF_ERROR("Event subsystem is already initialized.");
				return FALSE;
		}
		sfmemset(&state, 0, sizeof(state));
		is_initialized = TRUE;
		return TRUE;
}

void event_shutdown() {
		for (u32 i = 0; i < MAX_EVENT_CODES; ++i) {
				if (state.registered[i].events) {
						vector_destroy(state.registered[i].events);
						state.registered[i].events = (void *)0; // long for NULL
				}
		}
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event) {
		if (!is_initialized) {
				SF_ERROR("Attempted to register an event with code %d before "
						 "the event "
						 "subsystem is initialized.",
						 code);
				return FALSE;
		}

		if (!state.registered[code].events) {
				state.registered[code].events = vector_create(registered_event);
		}
		u64 registered_count = vector_len(state.registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				if (state.registered[code].events[i].listener == listener) {
						SF_WARNING("Tried to register the same listener twice "
								   "for event code %d",
								   code);
						return FALSE;
				}
		}

		registered_event event;
		event.listener = listener;
		event.callback = on_event;
		vector_push(state.registered[code].events, event);
		return TRUE;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event) {
		if (!is_initialized) {
				SF_ERROR("Attempted to unregister an event with code %d before "
						 "the event "
						 "subsystem is initialized.",
						 code);
				return FALSE;
		}
		if (!state.registered[code].events) {
				SF_WARNING("There are no events registered with code %d", code);
				return FALSE;
		}
		u64 registered_count = vector_len(state.registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				registered_event e = state.registered[code].events[i];
				if (e.listener == listener && e.callback == on_event) {
						registered_event popped;
						vector_pop_at(state.registered[code].events, i,
									  &popped);
						return TRUE;
				}
		}
		return FALSE;
}

b8 event_fire(u16 code, void *sender, event_context context) {
		if (!is_initialized) {
				SF_ERROR(
					"Attempted to fire an event with code %d before the event "
					"subsystem is "
					"initialized.");
				return FALSE;
		}
		if (!state.registered[code].events) {
				return FALSE;
		}
		u64 registered_count = vector_len(state.registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				registered_event e = state.registered[code].events[i];
				if (e.callback(code, sender, e.listener, context)) {
						// Event consumed, do not send to others.
						return TRUE;
				}
		}
		return FALSE;
}
