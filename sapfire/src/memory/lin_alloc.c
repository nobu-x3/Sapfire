#include "core/logger.h"
#include "core/sfmemory.h"
#include "lin_alloc.h"

void linear_allocator_create(u64 total_size, void *memory,
							 linear_allocator *out_allocator) {
		if (out_allocator) {
				out_allocator->total_size = total_size;
				out_allocator->allocated = 0;
				out_allocator->is_owner = memory == SF_NULL;
				if (memory) {
						out_allocator->mem_block = memory;
				} else {
						out_allocator->mem_block =
							sfalloc(total_size, MEMORY_TAG_LIN_ALLOC);
				}
		}
}
void linear_allocator_destroy(linear_allocator *allocator) {
		if (allocator) {
				if (allocator->is_owner && allocator->mem_block) {
						sffree(allocator->mem_block, allocator->total_size,
							   MEMORY_TAG_LIN_ALLOC);
				} else {
						allocator->mem_block =
							SF_NULL; // NOTE: responsibility of the owner to
									 // clean up.
				}
				allocator->total_size = 0;
				allocator->allocated = 0;
				allocator->is_owner = FALSE;
		}
}

void *linear_allocator_alloc(linear_allocator *allocator, u64 size) {
		if (allocator && allocator->mem_block) {
				if (allocator->allocated + size > allocator->total_size) {
						u64 remaining =
							allocator->total_size - allocator->allocated;
						SF_ERROR("LIN_ALLOC_ERROR: Overflow. Remaining memory: "
								 "%lluB, passed size: %lluB. Returning NULL.",
								 remaining, size);
						return SF_NULL;
				}
				void *block = allocator->mem_block + allocator->allocated;
				allocator->allocated += size;
				return block;
		}
		SF_ERROR("LIN_ALLOC_ERROR: allocator uninitialized.");
		return SF_NULL;
}
void linear_allocator_clear(linear_allocator *allocator) {
		if (allocator && allocator->mem_block) {
				allocator->allocated = 0;
				sfmemset(allocator->mem_block, 0, allocator->total_size);
		}
}
