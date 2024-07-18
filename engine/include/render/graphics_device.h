#pragma once

#include <dxgi.h>
#include <dxgi1_6.h>
#include "core/core.h"
#include "core/stl/unique_ptr.h"
#include "d3d12.h"
#include "dxgiformat.h"
#include "render/command_queue.h"
#include "render/compute_context.h"
#include "render/copy_context.h"
#include "render/d3d_util.h"
#include "render/memory_allocator.h"
#include "render/pipeline_state.h"
#include "render/resources.h"

namespace Sapfire::d3d {

	class DescriptorHeap;
	class GraphicsContext;

	struct FenceValues {
		u64 direct_queue_fence_value{};
	};

	struct SwapchainCreationDesc {
		u32 width, height;
		u32 refresh_rate;
		u32 buffer_count;
		DXGI_FORMAT swapchain_format;
		HWND output_window;
	};

	class SFAPI GraphicsDevice {
	public:
		explicit GraphicsDevice(const SwapchainCreationDesc& desc);
		~GraphicsDevice();
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;
		GraphicsDevice(GraphicsDevice&&) = delete;
		GraphicsDevice& operator=(GraphicsDevice&&) = delete;

		CommandQueue* direct_command_queue() const;
		CommandQueue* compute_command_queue() const;
		ID3D12Device* device() const;
		DescriptorHeap* cbv_srv_uav_descriptor_heap() const;
		DescriptorHeap* sampler_descriptor_heap() const;
		DescriptorHeap* dsv_descriptor_heap() const;
		DescriptorHeap* rtv_descriptor_heap() const;
		// TODO: mipmap generator
		DXGI_FORMAT swapchain_back_buffer_format() const;
		[[nodiscard]] stl::unique_ptr<GraphicsContext>& current_graphics_contexts();
		[[nodiscard]] stl::unique_ptr<CopyContext>& copy_context();
		[[nodiscard]] Texture& current_back_buffer();
		[[nodiscard]] stl::unique_ptr<ComputeContext> compute_context();
		void execute_and_flush_compute_context(stl::unique_ptr<ComputeContext>&& context);

		// Resets current context
		void begin_frame();
		// Signals direct cmd queue, awaits for execution of commands for next frame
		void end_frame();
		void present();
		void resize_window(u32 width, u32 height);

		// Creates a GPU buffer. If data is passed, makes a recursive call to create an upload buffer and issues a copy command. The
		// returned buffer is GPU exclusive memory which has the required data.
		template <typename T>
		[[nodiscard]] Buffer create_buffer(const BufferCreationDesc& desc, stl::span<const T> data = {}) const;

		// Creates texture on GPU. Used for all textures: render target, depth stencil target, etc. If data is passed, will use a texture
		// importer to load it, and an upload buffer is created and populated.
		[[nodiscard]] Texture create_texture(const TextureCreationDesc& desc, const void* data = nullptr);

		// TODO: create_sampler(...)

		[[nodiscard]] PipelineState create_pipeline_state(const GraphicsPipelineStateCreationDesc& desc) const;

		[[nodiscard]] PipelineState create_pipeline_state(const ComputePipelineStateCreationDesc& desc) const;
		[[nodiscard]] inline u64 current_frame_id() const { return m_CurrentFrameId; }

	private:
		void init_device_resources();
		void init_swapchain_resources(u32 width, u32 height);
		void init_directx();
		void init_command_queues();
		void init_descriptor_heaps();
		void init_memory_allocator();
		void init_contexts();
		void init_bindless_root_signature();
		// TODO: void init_mimap_generator();
		void create_backbuffer_rtvs();
		u32 create_cbv(const CbvCreationDesc& desc) const;
		u32 create_srv(const SrvCreationDesc& desc, ID3D12Resource* resource) const;
		u32 create_uav(const UavCreationDesc& desc, ID3D12Resource* resource) const;
		u32 create_rtv(const RtvCreationDesc& desc, ID3D12Resource* resource) const;
		u32 create_dsv(const DsvCreationDesc& desc, ID3D12Resource* resource) const;

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory{};
		Microsoft::WRL::ComPtr<ID3D12Debug3> m_Debug{};
		Microsoft::WRL::ComPtr<ID3D12DebugDevice> m_DebugDevice{};
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_Adapter{};
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device{};
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_Swapchain{};
		stl::unique_ptr<CommandQueue> m_DirectCommandQueue{};
		stl::unique_ptr<CommandQueue> m_CopyCommandQueue{};
		stl::unique_ptr<CommandQueue> m_ComputeCommandQueue{};
		stl::array<stl::unique_ptr<GraphicsContext>, MAX_FRAMES_IN_FLIGHT> m_PerFrameGraphicsContexts{};
		stl::queue<stl::unique_ptr<ComputeContext>> m_ComputeContextQueue{};
		stl::unique_ptr<CopyContext> m_CopyContext{};
		stl::unique_ptr<ComputeContext> m_ComputeContext{};
		stl::array<FenceValues, MAX_FRAMES_IN_FLIGHT> m_FenceValues;
		stl::array<Texture, MAX_FRAMES_IN_FLIGHT> m_BackBuffers{};
		u64 m_CurrentFrameId{};
		DXGI_FORMAT m_SwapchainBackbufferFormat{};
		HWND m_WindowHandle{};
		stl::unique_ptr<DescriptorHeap> m_RtvDescriptorHeap{};
		stl::unique_ptr<DescriptorHeap> m_DsvDescriptorHeap{};
		stl::unique_ptr<DescriptorHeap> m_CbvSrvUavDescriptorHeap{};
		stl::unique_ptr<DescriptorHeap> m_SamplerDescriptorHeap{};
		stl::unique_ptr<MemoryAllocator> m_MemoryAllocator{};
		/* stl::unique_ptr<MipMapGenerator> m_MipMapGenerator{}; */
		mutable stl::recursive_mutex m_ResourceMutex{};
		bool m_IsInitialized{false};
	};

	template <typename T>
	Buffer GraphicsDevice::create_buffer(const BufferCreationDesc& desc, stl::span<const T> data /*= {}*/) const {
		Buffer buffer{};
		// size == 0 means we're passing the data later through update functions
		const u32 num_components = data.size() == 0 ? 1 : static_cast<u32>(data.size());
		buffer.size_in_bytes = desc.usage == BufferUsage::ConstantBuffer
			? buffer.size_in_bytes = calculate_constant_buffer_byte_size(sizeof(T)) * num_components
			: sizeof(T) * num_components;
		const ResourceCreationDesc resource_desc = ResourceCreationDesc::create_buffer_resource_creation_desc(buffer.size_in_bytes);
		buffer.allocation = m_MemoryAllocator->allocate_buffer_resource(desc, resource_desc);
		stl::scoped_lock<stl::recursive_mutex> lock(m_ResourceMutex);
		// TODO: consider using backing storage for upload contexts. This just uses D3D12MA to do all ops.
		if (data.data()) {
			auto upload_buffer_name = L"Upload Buffer - " + stl::wstring(desc.name);
			const BufferCreationDesc upload_buffer_desc{
				.usage = BufferUsage::UploadBuffer,
				.name = upload_buffer_name,
			};
			Allocation upload_alloc = m_MemoryAllocator->allocate_buffer_resource(upload_buffer_desc, resource_desc);
			upload_alloc.update(data.data(), buffer.size_in_bytes);
			m_CopyContext->reset_context();
			m_CopyContext->command_list()->CopyResource(buffer.allocation.resource.Get(), upload_alloc.resource.Get());
			stl::array<const Context*, 1> contexts{m_CopyContext.get()};
			m_CopyCommandQueue->execute_context(contexts);
			m_CopyCommandQueue->flush();
			upload_alloc.reset();
		}
		if (desc.usage == BufferUsage::StructuredBuffer) {
			// Clang bs won't let me initialize with designated initializer
			const SrvCreationDesc srv_desc = {.srv_desc = D3D12_SHADER_RESOURCE_VIEW_DESC(
												  DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_BUFFER, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
												  {
													  .Buffer =
														  {
															  .FirstElement = 0,
															  .NumElements = static_cast<UINT>(data.size()),
															  .StructureByteStride = static_cast<UINT>(sizeof(T)),
															  .Flags = D3D12_BUFFER_SRV_FLAG_NONE,
														  },
												  })};
			buffer.srv_index = create_srv(srv_desc, buffer.allocation.resource.Get());
		} else if (desc.usage == BufferUsage::ConstantBuffer) {
			const CbvCreationDesc cbv_desc = {
				.cbv_desc =
					{
						.BufferLocation = buffer.allocation.resource->GetGPUVirtualAddress(),
						.SizeInBytes = static_cast<UINT>(buffer.size_in_bytes),
					},
			};
			buffer.cbv_index = create_cbv(cbv_desc);
		}
		return buffer;
	}
} // namespace Sapfire::d3d
