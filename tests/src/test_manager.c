#include "test_manager.h"
#include <containers/vector.h>
#include <core/clock.h>
#include <stdio.h>
#include <string.h>

typedef struct test_entry {
		PFN_test func;
		char *desc;
} test_entry;

static test_entry *tests;

void test_manager_init() { tests = vector_create(test_entry); }

void test_manager_register_test(u8 (*PFN_test)(), char *desc) {
		test_entry entry = {PFN_test, desc};
		vector_push(tests, entry);
}

void test_manager_run_tests() {
		u32 passed = 0;
		u32 failed = 0;
		u32 skipped = 0;
		u32 len = vector_len(tests);
		clock total_time;
		clock_start(&total_time);

		for (u32 i = 0; i < len; ++i) {
				clock test_time;
				clock_start(&test_time);
				u8 result = tests[i].func();
				clock_tick(&test_time);
				if (result == TRUE) {
						passed++;
				} else if (result == FALSE) {
						SF_FATAL("[FAILED]: %s", tests[i].desc);
						failed++;
				} else {
						SF_WARNING("[SKIPPED]: %s", tests[i].desc);
				}
				char status[20];
				i32 written = sprintf(status,
									  failed ? "====== %d FAILED ======"
											 : "====== SUCCESS ======",
									  failed);
				clock_tick(&total_time);
				SF_INFO("%d out of %d tests executed, skipped %d.(%.4f sec "
						"/ %.4f sec total)\n %s",
						i + 1, len, skipped, status, test_time.elapsed_ticks,
						total_time.elapsed_ticks);
		}
		clock_stop(&total_time);
		SF_INFO("Result: %d passed, %d failed, %d skipped", passed, failed,
				skipped);
}
