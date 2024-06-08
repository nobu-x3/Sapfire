#include "core/core.h"
#include "engpch.h"

#include <d3d12.h>
#include <dxgi.h>
#include "D3D12MemAlloc.h"
#include "render/memory_allocator.h"
#include "render/resources.h"

namespace Sapfire::d3d {
	MemoryAllocator::MemoryAllocator(ID3D12Device* device, IDXGIAdapter* adapter) {
		const D3D12MA::ALLOCATOR_DESC desc{
			.pDevice = device,
			.pAdapter = adapter,
		};
		d3d_check(D3D12MA::CreateAllocator(&desc, &m_Allocator));
	}

	Allocation MemoryAllocator::allocate_buffer_resource(const BufferCreationDesc& buffer_desc, const ResourceCreationDesc& resource_desc) {
		Allocation allocation{};
		D3D12_RESOURCE_STATES resource_state{};
		D3D12_HEAP_TYPE heap_type{};
		bool is_cpu_visible = false;
		switch (buffer_desc.usage) {
		case BufferUsage::UploadBuffer:
		case BufferUsage::ConstantBuffer:
			{
				// GenericRead implies readable data from the GPU memory. Required resourceState for upload heaps.
				// UploadHeap : CPU writable access, GPU readable access.
				resource_state = D3D12_RESOURCE_STATE_GENERIC_READ;
				heap_type = D3D12_HEAP_TYPE_UPLOAD;
				is_cpu_visible = true;
			}
			break;
		case BufferUsage::IndexBuffer:
		case BufferUsage::StructuredBuffer:
			{
				resource_state = D3D12_RESOURCE_STATE_COMMON;
				heap_type = D3D12_HEAP_TYPE_DEFAULT;
				is_cpu_visible = false;
			}
			break;
		}
		const D3D12MA::ALLOCATION_DESC allocation_desc{
			.HeapType = heap_type,
		};
		stl::lock_guard<stl::recursive_mutex> allocation_lock(m_ResourceAllocationMutex);
		d3d_check(m_Allocator->CreateResource(&allocation_desc, &resource_desc.resource_desc, resource_state, nullptr,
											  &allocation.allocation, IID_PPV_ARGS(&allocation.resource)));
		if (is_cpu_visible) {
			allocation.mapped_data = nullptr;
			d3d_check(allocation.resource->Map(0, nullptr, &allocation.mapped_data.value()));
		}
		if (buffer_desc.name.size() > 0) {
			allocation.resource->SetName(buffer_desc.name.data());
		}
		allocation.allocation->SetResource(allocation.resource.Get());
		return allocation;
	}

	Allocation MemoryAllocator::allocate_texture_resource(const TextureCreationDesc& texture_desc) {
		Allocation allocation{};
		DXGI_FORMAT format = texture_desc.format;
		DXGI_FORMAT ds_format{};
		switch (texture_desc.format) {
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
			{
				ds_format = DXGI_FORMAT_D32_FLOAT;
				format = DXGI_FORMAT_R32_FLOAT;
			}
			break;
		default:
			break;
		}
		ResourceCreationDesc resource_creation_desc{
			.resource_desc =
				{
					.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
					.Alignment = 0u,
					.Width = texture_desc.width,
					.Height = texture_desc.height,
					.DepthOrArraySize = static_cast<UINT16>(texture_desc.depthOrArraySize),
					.MipLevels = static_cast<UINT16>(texture_desc.mipLevels),
					.Format = format,
					.SampleDesc =
						{
							.Count = 1u,
							.Quality = 0u,
						},
					.Flags = D3D12_RESOURCE_FLAG_NONE,
				},
		};
		if (resource_creation_desc.resource_desc.MipLevels >= resource_creation_desc.resource_desc.Width) {
			resource_creation_desc.resource_desc.MipLevels = static_cast<UINT16>(resource_creation_desc.resource_desc.Width - 1);
		}
		if (resource_creation_desc.resource_desc.MipLevels >= resource_creation_desc.resource_desc.Height) {
			resource_creation_desc.resource_desc.MipLevels = static_cast<UINT16>(resource_creation_desc.resource_desc.Height - 1);
		}
		const u32 mip_levels = resource_creation_desc.resource_desc.MipLevels;
		// Since we copy data from upload buffer to texture with UpdateSubresource, we do not care about the heap's state, so can be
		// STATE_COMMON and HEAP_DEFAULT. Depth stencil are set to DEPTH_WRITE to avoid validation errors.
		D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;
		constexpr D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT;
		D3D12MA::ALLOCATION_DESC allocation_desc{
			.HeapType = heap_type,
		};
		switch (texture_desc.usage) {
		case TextureUsage::DepthStencil:
			{
				resource_creation_desc.resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				resource_creation_desc.resource_desc.Format = ds_format;
				// Since it's a large resource, it's better to for it to be committed and have it's own heap
				allocation_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
				resource_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}
			break;
		case TextureUsage::RenderTarget:
			{
				resource_creation_desc.resource_desc.Flags =
					D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				allocation_desc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
				allocation_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
				resource_state = texture_desc.optional_initial_state.has_value() ? texture_desc.optional_initial_state.value()
																				 : D3D12_RESOURCE_STATE_RENDER_TARGET;
			}
			break;
		case TextureUsage::TextureFromData:
		case TextureUsage::TextureFromPath:
		case TextureUsage::HDRTextureFromPath:
		case TextureUsage::CubeMap:
		case TextureUsage::UAVTexture:
			{
				resource_creation_desc.resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			break;
		};
		stl::optional<D3D12_CLEAR_VALUE> optimized_clear_value{};
		if (texture_desc.usage == TextureUsage::RenderTarget) {
			optimized_clear_value = {
				.Format = format,
				.Color = {0.0f, 0.0f, 0.0f, 1.0f},
			};
		} else if (texture_desc.usage == TextureUsage::DepthStencil) {
			constexpr D3D12_DEPTH_STENCIL_VALUE ds_value{
				.Depth = 1.0f,
				.Stencil = 1,
			};
			optimized_clear_value = {
				.Format = ds_format,
				.DepthStencil = ds_value,
			};
		}
		stl::lock_guard<stl::recursive_mutex> alloc_guard(m_ResourceAllocationMutex);
		if (texture_desc.optional_initial_state.has_value() && texture_desc.optional_initial_state != D3D12_RESOURCE_STATE_COMMON) {
			resource_state = texture_desc.optional_initial_state.value();
		}
		d3d_check(m_Allocator->CreateResource(&allocation_desc, &resource_creation_desc.resource_desc, resource_state,
											  optimized_clear_value.has_value() ? &optimized_clear_value.value() : nullptr,
											  &allocation.allocation, IID_PPV_ARGS(&allocation.resource)));
		allocation.resource->SetName(texture_desc.name.data());
		allocation.allocation->SetResource(allocation.resource.Get());
		return allocation;
	}
} // namespace Sapfire::d3d
