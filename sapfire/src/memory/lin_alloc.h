#pragma once
#include "defines.h"

typedef struct linear_allocator{
	u64 total_size;
	u64 allocated;
	void* mem_block;
	b8 is_owner;
} linear_allocator;

SAPI void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator);
SAPI void linear_allocator_destroy(linear_allocator* allocator);
SAPI void* linear_allocator_alloc(linear_allocator* allocator, u64 size);
SAPI void linear_allocator_clear(linear_allocator* allocator);
