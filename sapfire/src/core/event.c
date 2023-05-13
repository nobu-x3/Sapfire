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
		b8 initialized;
} event_system_state;

static event_system_state *pState;

b8 event_initialize(u64 *mem_size, void *mem_block) {
		*mem_size = sizeof(event_system_state);
		if (mem_block == SF_NULL) {
				return FALSE;
		}
		pState = mem_block;
		pState->initialized = TRUE;
		// sfmemset(&pState, 0, sizeof(event_system_state));
		SF_INFO("Event subsystem initialized successfully.");
		return TRUE;
}

void event_shutdown(void *memory) {
		for (u32 i = 0; i < MAX_EVENT_CODES; ++i) {
				if (pState->registered[i].events) {
						vector_destroy(pState->registered[i].events);
						pState->registered[i].events =
							(void *)0; // long for NULL
				}
		}
		pState = SF_NULL;
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event) {
		if (!pState->initialized) {
				SF_ERROR("Attempted to register an event with code %d before "
						 "the event "
						 "subsystem is initialized.",
						 code);
				return FALSE;
		}

		if (!pState->registered[code].events) {
				pState->registered[code].events =
					vector_create(registered_event);
		}
		u64 registered_count = vector_len(pState->registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				if (pState->registered[code].events[i].listener == listener) {
						SF_WARNING("Tried to register the same listener twice "
								   "for event code %d",
								   code);
						return FALSE;
				}
		}

		registered_event event;
		event.listener = listener;
		event.callback = on_event;
		vector_push(pState->registered[code].events, event);
		return TRUE;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event) {
		if (!pState->initialized) {
				SF_ERROR("Attempted to unregister an event with code %d before "
						 "the event "
						 "subsystem is initialized.",
						 code);
				return FALSE;
		}
		if (!pState->registered[code].events) {
				SF_WARNING("There are no events registered with code %d", code);
				return FALSE;
		}
		u64 registered_count = vector_len(pState->registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				registered_event e = pState->registered[code].events[i];
				if (e.listener == listener && e.callback == on_event) {
						registered_event popped;
						vector_pop_at(pState->registered[code].events, i,
									  &popped);
						return TRUE;
				}
		}
		return FALSE;
}

b8 event_fire(u16 code, void *sender, event_context context) {
		if (!pState->initialized) {
				SF_ERROR(
					"Attempted to fire an event with code %d before the event "
					"subsystem is "
					"initialized.");
				return FALSE;
		}
		if (!pState->registered[code].events) {
				return FALSE;
		}
		u64 registered_count = vector_len(pState->registered[code].events);
		for (u64 i = 0; i < registered_count; ++i) {
				registered_event e = pState->registered[code].events[i];
				if (e.callback(code, sender, e.listener, context)) {
						// Event consumed, do not send to others.
						return TRUE;
				}
		}
		return FALSE;
}
