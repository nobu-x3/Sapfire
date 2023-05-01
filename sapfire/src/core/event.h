#pragma once

#include "defines.h"

typedef struct event_context{
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


typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listener_list, event_context data);

b8 event_initialize();
void event_shutdown();

SAPI b8 event_register(u16 code, void* listener, PFN_on_event on_event);
SAPI b8 event_unregister(u16 code, void* listener, PFN_on_event on_event);
SAPI b8 event_fire(u16 code, void* sender, event_context context);

// Internal event codes. Application codes should use codes beyond 255.
typedef enum event_code{
	EVENT_CODE_APPLICATION_QUIT = 0x01,

	MAX_EVENT_CODE = 0xFF
} event_code;
