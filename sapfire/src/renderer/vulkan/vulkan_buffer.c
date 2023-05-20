#include "core/logger.h"
#include "core/sfmemory.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_buffer.h"

b8 vulkan_buffer_create(vulkan_context *context, u64 size,
						VkBufferUsageFlagBits usage, u32 mem_props,
						vulkan_buffer *out_buffer) {
		sfmemset(out_buffer, 0, sizeof(vulkan_buffer));
		out_buffer->size = size;
		out_buffer->usage = usage;
		out_buffer->mem_prop_flags = mem_props;

		VkBufferCreateInfo buffer_ci = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		buffer_ci.size = size;
		buffer_ci.usage = usage;
		buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT_SUCCESS(vkCreateBuffer(context->device.logical_device,
										 &buffer_ci, context->allocator,
										 &out_buffer->handle),
						  "Failed to create buffer.");

		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(context->device.logical_device,
									  out_buffer->handle, &mem_reqs);
		out_buffer->mem_index =
			context->find_memory_index(mem_reqs.memoryTypeBits, mem_props);
		if (out_buffer->mem_index == -1) {
				SF_ERROR("Failed to create buffer due to incompatible memory "
						 "requirements.");
				return FALSE;
		}
		VkMemoryAllocateInfo mem_alloc_info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		mem_alloc_info.memoryTypeIndex = out_buffer->mem_index;
		mem_alloc_info.allocationSize = mem_reqs.size;
		VK_ASSERT_SUCCESS(vkAllocateMemory(context->device.logical_device,
										   &mem_alloc_info, context->allocator,
										   &out_buffer->device_mem),
						  "Failed to allocate memory for buffer.");
		return TRUE;
}

void vulkan_buffer_destroy(vulkan_context *context, vulkan_buffer *buffer) {
		if (buffer->device_mem) {
				vkFreeMemory(context->device.logical_device, buffer->device_mem,
							 context->allocator);
				buffer->device_mem = SF_NULL;
		}
		if (buffer->handle) {
				vkDestroyBuffer(context->device.logical_device, buffer->handle,
								context->allocator);
		}
		buffer->size = 0;
		buffer->usage = 0;
		buffer->is_mapped = FALSE;
}

b8 vulkan_buffer_resize(vulkan_context *context, u64 new_size,
						vulkan_buffer *buffer, VkQueue queue,
						VkCommandPool cmd_pool) {

		VkBufferCreateInfo buffer_ci = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		buffer_ci.size = new_size;
		buffer_ci.usage = buffer->usage;
		buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer new_buffer;
		VK_ASSERT_SUCCESS(
			vkCreateBuffer(context->device.logical_device, &buffer_ci,
						   context->allocator, &new_buffer),
			"Failed to resize buffer because failed to create buffer.");

		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(context->device.logical_device,
									  new_buffer, &mem_reqs);

		VkDeviceMemory new_mem;
		VkMemoryAllocateInfo mem_alloc_info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		mem_alloc_info.memoryTypeIndex = buffer->mem_index;
		mem_alloc_info.allocationSize = mem_reqs.size;
		VK_ASSERT_SUCCESS(vkAllocateMemory(context->device.logical_device,
										   &mem_alloc_info, context->allocator,
										   &new_mem),
						  "Failed to resize buffer beacuse failed to allocate "
						  "memory for buffer.");
		VK_ASSERT_SUCCESS(
			vkBindBufferMemory(context->device.logical_device, new_buffer,
							   new_mem, 0),
			"Failed to resize buffer because failed to bind the new buffer.");
		vulkan_buffer_copy(context, queue, cmd_pool, buffer->handle, 0,
						   new_buffer, 0, buffer->size);
		vkDeviceWaitIdle(context->device.logical_device);
		if (buffer->device_mem) {
				vkFreeMemory(context->device.logical_device, buffer->device_mem,
							 context->allocator);
				buffer->device_mem = SF_NULL;
		}
		if (buffer->handle) {
				vkDestroyBuffer(context->device.logical_device, buffer->handle,
								context->allocator);
				buffer->handle = SF_NULL;
		}
		buffer->size = new_size;
		buffer->handle = new_buffer;
		buffer->device_mem = new_mem;
		return TRUE;
}

void vulkan_buffer_copy(vulkan_context *context, VkQueue queue,
						VkCommandPool cmd_pool, VkBuffer src, u64 src_offset,
						VkBuffer dest, u64 dest_offset, u64 size) {
		vkDeviceWaitIdle(context->device.logical_device);
		vulkan_command_buffer temp;
		vulkan_command_buffer_alloc_and_begin_single_use(context, cmd_pool,
														 &temp);
		VkBufferCopy copy_region;
		copy_region.srcOffset = src_offset;
		copy_region.dstOffset = dest_offset;
		copy_region.size = size;
		vkCmdCopyBuffer(temp.handle, src, dest, 1, &copy_region);
		vulkan_command_buffer_end_single_use(context, cmd_pool, &temp, queue);
}

void vulkan_buffer_bind(vulkan_context *context, vulkan_buffer *buffer,
						u64 offset) {
		VK_ASSERT_SUCCESS(vkBindBufferMemory(context->device.logical_device,
											 buffer->handle, buffer->device_mem,
											 offset),
						  "Failed to bind buffer.");
}

void *vulkan_buffer_map(vulkan_context *context, vulkan_buffer *buffer,
						u64 size, u64 offset, u32 flags) {
		void *data;
		VK_ASSERT_SUCCESS(vkMapMemory(context->device.logical_device,
									  buffer->device_mem, offset, size, flags,
									  &data),
						  "Failed to map memory.");
		return data;
}

void vulkan_buffer_unmap(vulkan_context *context, vulkan_buffer *buffer) {
		vkUnmapMemory(context->device.logical_device, buffer->device_mem);
}

void vulkan_buffer_load_data(vulkan_context *context, vulkan_buffer *buffer,
							 u64 size, u64 offset, u32 flags,
							 const void *data) {
		void *pData = vulkan_buffer_map(context, buffer, size, offset, flags);
		sfmemcpy(pData, data, size);
		vulkan_buffer_unmap(context, buffer);
}
