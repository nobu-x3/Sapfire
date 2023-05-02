#include "clock.h"
#include "platform/platform.h"

f64 clock_get_delta_time_seconds(clock *c) {
		return (c->current_tick - c->last_tick) / 1000.0f;
}

u64 clock_get_delta_time_ms(clock *c) {
		return (c->current_tick - c->last_tick) * 1000;
}

void clock_tick(clock *c) {
		c->last_tick = c->current_tick;
		c->current_tick = platform_get_absolute_time();
}
