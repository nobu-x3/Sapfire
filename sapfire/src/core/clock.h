#pragma once

#include "defines.h"

typedef struct clock{
	u64 last_tick;
	u64 current_tick;
} clock;

f64 clock_get_delta_time_seconds(clock* c);
u64 clock_get_delta_time_ms(clock* c);
void clock_tick(clock* c);
