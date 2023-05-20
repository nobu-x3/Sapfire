#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkan_types.h"

b8 vulkan_buffer_create(vulkan_context *context, u64 size,
                        VkBufferUsageFlagBits usage, u32 mem_props,
                        vulkan_buffer *out_buffer);
void vulkan_buffer_destroy(vulkan_context *context, vulkan_buffer *buffer);
b8 vulkan_buffer_resize(vulkan_context *context, u64 new_size,
                        vulkan_buffer *buffer, VkQueue queue,
                        VkCommandPool cmd_pool);
void vulkan_buffer_copy(vulkan_context *context, VkQueue queue,
                        VkCommandPool cmd_pool, VkBuffer src, u64 src_offset,
                        VkBuffer dest, u64 dest_offset, u64 size);
void vulkan_buffer_load_data(vulkan_context *context, vulkan_buffer *buffer,
                             u64 size, u64 offset, u32 flags, const void *data);
void vulkan_buffer_bind(vulkan_context *context, vulkan_buffer *buffer,
                        u64 offset);
void *vulkan_buffer_map(vulkan_context *context, vulkan_buffer *buffer,
                        u64 size, u64 offset, u32 flags);
void vulkan_buffer_unmap(vulkan_context *context, vulkan_buffer *buffer);