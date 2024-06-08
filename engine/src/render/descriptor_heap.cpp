#include "d3d12.h"
#include "engpch.h"

#include "render/descriptor_heap.h"

namespace Sapfire::d3d {

	DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, u32 descriptor_count,
								   stl::wstring_view heap_name) {
		const D3D12_DESCRIPTOR_HEAP_FLAGS flags =
			(heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			? D3D12_DESCRIPTOR_HEAP_FLAG_NONE
			: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		const D3D12_DESCRIPTOR_HEAP_DESC desc{
			.Type = heap_type,
			.NumDescriptors = descriptor_count,
			.Flags = flags,
			.NodeMask = 0,
		};
		d3d_check(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DescriptorHeap)));
		m_DescriptorHeap->SetName(heap_name.data());
		m_DescriptorSize = device->GetDescriptorHandleIncrementSize(heap_type);
		m_DescriptorHandleFromHeapStart = {
			.cpu_descriptor_handle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			.gpu_descriptor_handle = flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
				? m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart()
				: CD3DX12_GPU_DESCRIPTOR_HANDLE{},
			.descriptor_size = m_DescriptorSize,
		};
		m_CurrentDescriptorHandle = m_DescriptorHandleFromHeapStart;
	}

	DescriptorHandle DescriptorHeap::descriptor_handle_from_index(u32 index) const {
		DescriptorHandle handle = descriptor_handle_from_start();
		offset_descriptor(handle, index);
		return handle;
	}

	u32 DescriptorHeap::descriptor_index(const DescriptorHandle& handle) const {
		return static_cast<u32>((handle.gpu_descriptor_handle.ptr - m_DescriptorHandleFromHeapStart.gpu_descriptor_handle.ptr) /
								m_DescriptorSize);
	}

	u32 DescriptorHeap::current_descriptor_index() const { return descriptor_index(m_CurrentDescriptorHandle); }

	void DescriptorHeap::offset_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE& handle, u32 offset /*= 1*/) const {
        handle.ptr += m_DescriptorSize * static_cast<size_t>(offset);
    }

	void DescriptorHeap::offset_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE& handle, u32 offset /*= 1*/) const{
        handle.ptr += m_DescriptorSize * static_cast<size_t>(offset);
    }

	void DescriptorHeap::offset_descriptor(DescriptorHandle& handle, u32 offset /*= 1*/) const{
        size_t offset_sizet = static_cast<size_t>(offset);
        handle.cpu_descriptor_handle.ptr += m_DescriptorSize * offset_sizet;
        handle.gpu_descriptor_handle.ptr += m_DescriptorSize * offset_sizet;
    }

	void DescriptorHeap::offset_current_handle(u32 offset /*= 1*/){
        offset_descriptor(m_CurrentDescriptorHandle, offset);
    }

} // namespace Sapfire::d3d
