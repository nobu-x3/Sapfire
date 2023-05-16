#pragma once
#include "defines.h"

typedef struct linear_allocator{
	u64 total_size;
	u64 allocated;
	void* mem_block;
	b8 is_owner;
} linear_allocator;

/**
* @brief Creates a linear allocator.
* @param total_size The size of the memory block to allocate.
* @param memory The memory block to transfer ownership of or NULL if none.
* @param out_allocator * The allocated block.
*/
SAPI void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator);

/**
* @brief Destroy a linear allocator. This frees all memory allocated by the allocator. It is safe to call this function more than once and will do nothing if it is the first call in a multi - threaded environment
* @param allocator * Pointer to the allocator
*/
SAPI void linear_allocator_destroy(linear_allocator* allocator);

/**
* @brief Allocate memory from the allocator. This function is called from the allocation function and can be used to allocate memory from the allocator.
* @param allocator * Pointer to the linear allocator. Must be initialized.
* @param size Size of memory to allocate in bytes. Must be less than or equal to mem_block
*/
SAPI void* linear_allocator_alloc(linear_allocator* allocator, u64 size);

/**
* @brief Clears the memory allocated by linear_allocator. This is useful when you want to re - use an allocator that was allocated with linear_allocator_new ().
* @param allocator The allocator to clear memory for. This must be non - NULL
*/
SAPI void linear_allocator_clear(linear_allocator* allocator);
