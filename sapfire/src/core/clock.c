#include "clock.h"
#include "platform/platform.h"

void clock_tick(clock *c) {
		if (c->start_tick != 0) {
				c->elapsed_ticks = platform_get_absolute_time() - c->start_tick;
		}
}

void clock_start(clock *c) {
		c->start_tick = platform_get_absolute_time();
		c->elapsed_ticks = 0;
}

void clock_stop(clock *c) { c->start_tick = 0; }
