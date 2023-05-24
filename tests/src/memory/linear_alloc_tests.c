#include "linear_alloc_tests.h"
#include "../expect.h"
#include "../test_manager.h"
#include <memory/lin_alloc.h>

u8 test_lin_alloc_create_and_destroy () {
	linear_allocator alloc;
	linear_allocator_create (sizeof (u64), SF_NULL, &alloc);
	expect_not (alloc.mem_block, SF_NULL);
	expect (alloc.total_size, sizeof (u64));
	expect (alloc.allocated, 0);

	linear_allocator_destroy (&alloc);
	expect (alloc.mem_block, SF_NULL);
	expect (alloc.total_size, 0);
	expect (alloc.allocated, 0);
	return 1;
}

u8 test_lin_alloc_multi_alloc_max_space () {
	u64 max_allocs = 2048;
	linear_allocator alloc;
	linear_allocator_create (sizeof (u64) * max_allocs, SF_NULL, &alloc);

	void *memblock;
	for (u64 i = 0; i < max_allocs; ++i) {
		memblock = linear_allocator_alloc (&alloc, sizeof (u64));
		expect_not (memblock, SF_NULL);
		expect (alloc.allocated, sizeof (u64) * (i + 1));
	}
	linear_allocator_destroy (&alloc);
	return TRUE;
}

u8 test_lin_alloc_overflow () {
	linear_allocator alloc;
	linear_allocator_create (sizeof (u64) * 4, SF_NULL, &alloc);

	void *memblock;
	for (u64 i = 0; i < 4; ++i) {
		memblock = linear_allocator_alloc (&alloc, sizeof (u64));
	}

	memblock = linear_allocator_alloc (&alloc, sizeof (u64));
	expect (memblock, SF_NULL);
	expect (alloc.allocated, sizeof (u64) * 4);
	linear_allocator_destroy (&alloc);
	return TRUE;
}

void lin_alloc_register_tests () {
	test_manager_register_test (
		test_lin_alloc_create_and_destroy,
		"Test creation and destruction of linear allocator");
	test_manager_register_test (test_lin_alloc_multi_alloc_max_space,
								"Test multiple allocations");
	test_manager_register_test (test_lin_alloc_overflow,
								"Test lin alloc overflow handling");
}