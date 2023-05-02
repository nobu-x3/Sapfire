#pragma once

#include "defines.h"
#include "platform/platform.h"

typedef struct clock{
	u64 start_tick;
	u64 elapsed_ticks;
} clock;

void clock_tick(clock* c);
void clock_start(clock* c);
void clock_stop(clock* c);