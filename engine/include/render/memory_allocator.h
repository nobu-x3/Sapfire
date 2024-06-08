#pragma once

#include <wrl/client.h>
#include "core/core.h"
#include "render/resources.h"

namespace D3D12MA {
	class Allocator;
}

class ID3D12Device;
class IDXGIAdapter;

namespace Sapfire::d3d {

	// Handles allocations of GPU memory. An allocation represents a heap allocated from GPU memory based on the heap type (GPU exclusive or
	// shared CPU/GPU memory)
	class SFAPI MemoryAllocator {
	public:
		explicit MemoryAllocator(ID3D12Device* device, IDXGIAdapter* adapter);
		~MemoryAllocator() = default;
		MemoryAllocator(const MemoryAllocator&) = delete;
		MemoryAllocator& operator=(const MemoryAllocator&) = delete;
		MemoryAllocator(MemoryAllocator&&) = delete;
		MemoryAllocator& operator=(MemoryAllocator&&) = delete;

		[[nodiscard]] Allocation allocate_buffer_resource(const BufferCreationDesc& buffer_desc, const ResourceCreationDesc& resource_desc);
		[[nodiscard]] Allocation allocate_texture_resource(const TextureCreationDesc& texture_desc);

	private:
		Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_Allocator{};
		stl::recursive_mutex m_ResourceAllocationMutex{};
	};

} // namespace Sapfire::d3d
