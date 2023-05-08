#include "platform/platform.h"
#include "sfmath.h"
#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = FALSE;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 sfsin(f32 x) { return sinf(x); }

f32 sfcos(f32 x) { return cosf(x); }

f32 sftan(f32 x) { return tanf(x); }

f32 sfacos(f32 x) { return acosf(x); }

f32 sfsqrt(f32 x) { return sqrtf(x); }

f32 sfabs(f32 x) { return fabsf(x); }

i32 sfrandom() {
		if (!rand_seeded) {
				srand((u32)platform_get_absolute_time());
				rand_seeded = TRUE;
		}
		return rand();
}

i32 sfrandom_in_range(i32 min, i32 max) {
		if (!rand_seeded) {
				srand((u32)platform_get_absolute_time());
				rand_seeded = TRUE;
		}
		return (rand() % (max - min + 1)) + min;
}

f32 sffrandom() { return (float)sfrandom() / (f32)RAND_MAX; }

f32 sffrandom_in_range(f32 min, f32 max) {
		return min + ((float)sfrandom() / ((f32)RAND_MAX / (max - min)));
}