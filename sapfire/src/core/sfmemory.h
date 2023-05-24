#pragma once

#include "defines.h"

typedef enum memory_tag {
	MEMORY_TAG_UNKNOWN,
	MEMORY_TAG_LIN_ALLOC,
	MEMORY_TAG_GAME,
	MEMORY_TAG_VECTOR,
	MEMORY_TAG_RENDERER,
	MEMORY_TAG_STRING,
	MEMORY_TAG_APPLICATION,

	MEMORY_TAG_MAX
} memory_tag;

/**
* @brief \ brief Initializes memory subsystem This function is called at boot time to initialize the memory subsystem. \ return
*/
void memory_initialize ();

/**
* @brief \ brief Print memory usage to debug log \ param [ in ] none \ param [ out ]
*/
void memory_shutdown ();

/**
* @brief Allocate memory with a tag.
* @param size Size of the block to allocate. Must be > = 0.
* @param tag Tag to use for the block. Use MEMORY_TAG_UNKNOWN for no tag
*/
SAPI void* sfalloc (u64 size, memory_tag tag);

/**
* @brief Free a block of memory.
* @param block Pointer to the block to free. Must be non - NULL.
* @param size Size of the block in bytes. Must be non - zero.
* @param tag Tag associated with the block. If tag == MEMORY_TAG_UNKNOWN then no tag is associated
*/
SAPI void sffree (void* block, u64 size, memory_tag tag);

/**
* @brief Copy memory from one location to another. This is a wrapper around platform_copy_memory () to avoid having to include platform code.
* @param dest The address to copy to. Must be aligned on 64 - bit boundaries.
* @param src The address to copy from. Must be aligned on 64 - bit boundaries.
* @param size The size in bytes of the memory region to copy
*/
SAPI void* sfmemcpy (void* dest, const void* src, u64 size);

/**
* @brief Set memory to a value. This is a wrapper around platform_set_memory to avoid having to worry having to include platform code.
* @param dest A pointer to the memory to be set
* @param val The value to be set
* @param size The size of the memory to be set in
*/
SAPI void* sfmemset (void* dest, i32 val, u64 size);

/**
* @brief Get memory usage as a string. This is used to display information about the process memory usage in a human readable format.
* @return pointer to string containing memory usage in a null - terminated string. Must be freed by the caller using
*/
SAPI char* get_mem_usage_str ();