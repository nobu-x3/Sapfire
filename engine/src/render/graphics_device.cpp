#include "engpch.h"

#include <combaseapi.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_6.h>
#include "core/core.h"
#include "core/file_system.h"
#include "core/logger.h"
#include "core/memory.h"
#include "d3d12.h"
#include "dxgiformat.h"
#include "render/command_queue.h"
#include "render/copy_context.h"
#include "render/d3d_util.h"
#include "render/descriptor_heap.h"
#include "render/graphics_context.h"
#include "render/graphics_device.h"
#include "render/memory_allocator.h"
#include "render/resources.h"
#include "tools/profiling.h"
#include "tools/texture_loader.h"

namespace Sapfire::d3d {
	GraphicsDevice::GraphicsDevice(const SwapchainCreationDesc& desc) :
		m_SwapchainBackbufferFormat(desc.swapchain_format), m_WindowHandle(desc.output_window) {
		PROFILE_FUNCTION();
		init_device_resources();
		init_swapchain_resources(desc.width, desc.height);
		m_IsInitialized = true;
	}

	GraphicsDevice::~GraphicsDevice() { m_DirectCommandQueue->flush(); }

	CommandQueue* GraphicsDevice::direct_command_queue() const { return m_DirectCommandQueue.get(); }

	CommandQueue* GraphicsDevice::compute_command_queue() const { return m_ComputeCommandQueue.get(); }

	ID3D12Device* GraphicsDevice::device() const { return m_Device.Get(); }

	DescriptorHeap* GraphicsDevice::cbv_srv_uav_descriptor_heap() const { return m_CbvSrvUavDescriptorHeap.get(); }

	DescriptorHeap* GraphicsDevice::sampler_descriptor_heap() const { return m_SamplerDescriptorHeap.get(); }

	DescriptorHeap* GraphicsDevice::dsv_descriptor_heap() const { return m_DsvDescriptorHeap.get(); }

	DescriptorHeap* GraphicsDevice::rtv_descriptor_heap() const { return m_RtvDescriptorHeap.get(); }

	DXGI_FORMAT GraphicsDevice::swapchain_back_buffer_format() const { return m_SwapchainBackbufferFormat; }

	stl::unique_ptr<GraphicsContext>& GraphicsDevice::current_graphics_contexts() { return m_PerFrameGraphicsContexts[m_CurrentFrameId]; }

	stl::unique_ptr<CopyContext>& GraphicsDevice::copy_context() { return m_CopyContext; }

	Texture& GraphicsDevice::current_back_buffer() { return m_BackBuffers[m_CurrentFrameId]; }

	stl::unique_ptr<ComputeContext> GraphicsDevice::compute_context() {
		PROFILE_FUNCTION();
		if (m_ComputeContextQueue.empty()) {
			stl::unique_ptr<ComputeContext> context = std::move(m_ComputeContextQueue.front());
			m_ComputeContextQueue.pop();
			return context;
		}
		stl::unique_ptr<ComputeContext> context = stl::make_unique<ComputeContext>(mem::ENUM::Engine_Rendering, this);
		return context;
	}

	void GraphicsDevice::execute_and_flush_compute_context(stl::unique_ptr<ComputeContext>&& context) {
		PROFILE_FUNCTION();
		stl::array<const Context*, 1> contexts = {context.get()};
		m_ComputeCommandQueue->execute_context(contexts);
		m_ComputeCommandQueue->flush();
		m_ComputeContextQueue.emplace(std::move(m_ComputeContext));
	}

	void GraphicsDevice::begin_frame() {
		PROFILE_FUNCTION();
		m_PerFrameGraphicsContexts[m_CurrentFrameId]->reset_context();
	}

	void GraphicsDevice::end_frame() {
		PROFILE_FUNCTION();
		m_FenceValues[m_CurrentFrameId].direct_queue_fence_value = m_DirectCommandQueue->signal();
		m_CurrentFrameId = m_Swapchain->GetCurrentBackBufferIndex();
		m_DirectCommandQueue->wait_for_fence_value(m_FenceValues[m_CurrentFrameId].direct_queue_fence_value);
	}

	void GraphicsDevice::present() {
		PROFILE_FUNCTION();
		d3d_check(m_Swapchain->Present(1, 0));
	}

	void GraphicsDevice::resize_window(u32 width, u32 height) {
		PROFILE_FUNCTION();
		m_DirectCommandQueue->flush();
		m_CopyCommandQueue->flush();
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			m_BackBuffers[i].allocation.resource.Reset();
			m_FenceValues[i].direct_queue_fence_value = m_DirectCommandQueue->current_completed_fence_value();
		}
		DXGI_SWAP_CHAIN_DESC swapchain_desc{};
		d3d_check(m_Swapchain->GetDesc(&swapchain_desc));
		d3d_check(m_Swapchain->ResizeBuffers(MAX_FRAMES_IN_FLIGHT, width, height, m_SwapchainBackbufferFormat, swapchain_desc.Flags));
		m_CurrentFrameId = m_Swapchain->GetCurrentBackBufferIndex();
		create_backbuffer_rtvs();
	}

	Texture GraphicsDevice::create_texture(const TextureCreationDesc& desc, const void* data /*= nullptr*/) {
		PROFILE_FUNCTION();
		Texture texture{};
		TextureCreationDesc texture_desc = desc;
		if (texture_desc.usage == TextureUsage::HDRTextureFromPath || texture_desc.usage == TextureUsage::TextureFromPath) {
			texture_desc.path = fs::full_path(texture_desc.path);
			if (texture_desc.path.empty()) {
				CORE_WARN("Texture at path {} could not be located by the filesystem. Taking a wild guess.", d3d::WStringToANSI(desc.path));
				texture_desc.path = d3d::AnsiToWString(fs::FileSystem::root_directory()) + desc.path;
			}
		}
		s32 width{};
		s32 height{};
		const s32 component_count{4};
		void* texture_data{(void*)(data)};
		f32* hdr_texture_data{nullptr};
		if (texture_desc.usage == TextureUsage::TextureFromData) {
			width = texture_desc.width;
			height = texture_desc.height;
			texture_data = const_cast<void*>(data);
		} else if (texture_desc.usage == TextureUsage::TextureFromPath) {
			texture_data = tools::texture_loader::load(WStringToANSI(texture_desc.path).c_str(), width, height, component_count);
			texture_desc.width = width;
			texture_desc.height = height;
		} else if (texture_desc.usage == TextureUsage::HDRTextureFromPath) {
			hdr_texture_data = tools::texture_loader::load_hdr(WStringToANSI(texture_desc.path).c_str(), width, height, component_count);
			texture_desc.width = width;
			texture_desc.height = height;
		}
		const stl::scoped_lock<stl::recursive_mutex> lock(m_ResourceMutex);
		texture.allocation = m_MemoryAllocator->allocate_texture_resource(texture_desc);
		texture.width = texture_desc.width;
		texture.height = texture_desc.height;
		const u32 mip_levels = texture_desc.mipLevels;
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
		};
		if (texture_data || hdr_texture_data) {
			auto buffer_name = L"Upload Buffer - " + stl::wstring(texture_desc.name);
			const BufferCreationDesc upload_buffer_creation_desc{
				.usage = BufferUsage::UploadBuffer,
				.name = buffer_name,
			};
			const u64 upload_buffer_size = GetRequiredIntermediateSize(texture.allocation.resource.Get(), 0, 1);
			const ResourceCreationDesc resource_creation_desc =
				ResourceCreationDesc::create_buffer_resource_creation_desc(upload_buffer_size);
			Allocation upload_allocation = m_MemoryAllocator->allocate_buffer_resource(upload_buffer_creation_desc, resource_creation_desc);
			const D3D12_SUBRESOURCE_DATA texture_subresource_data{
				.pData = desc.usage == TextureUsage::TextureFromPath || desc.usage == TextureUsage::TextureFromData ? texture_data
																													: hdr_texture_data,
				.RowPitch = width * texture_desc.bytesPerPixel,
				.SlicePitch = width * height * texture_desc.bytesPerPixel,
			};
			m_CopyContext->reset_context();
			UpdateSubresources(m_CopyContext->command_list(), texture.allocation.resource.Get(), upload_allocation.resource.Get(), 0, 0, 1,
							   &texture_subresource_data);
			stl::array<const Context*, 1> contexts{m_CopyContext.get()};
			m_CopyCommandQueue->execute_context(contexts);
			m_CopyCommandQueue->flush();
			upload_allocation.reset();
		}
		SrvCreationDesc srv_creation_desc{};
		if (texture_desc.depthOrArraySize == 1) {
			srv_creation_desc = {
				.srv_desc =
					{
						.Format = format,
						.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
						.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
						.Texture2D =
							{
								.MostDetailedMip = 0,
								.MipLevels = mip_levels,
							},
					},
			};
		} else if (texture_desc.depthOrArraySize == 6) {
			srv_creation_desc = {
				.srv_desc =
					{
						.Format = format,
						.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE,
						.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
						.Texture2D =
							{
								.MostDetailedMip = 0,
								.MipLevels = mip_levels,
							},
					},
			};
		}
		texture.srv_index = create_srv(srv_creation_desc, texture.allocation.resource.Get());
		// SRVs for mip levels, accesed through texture.srv_index + i
		if (texture_desc.mipLevels > 1 && texture_desc.usage == TextureUsage::UAVTexture) {
			for (u32 i = 1; i < texture_desc.mipLevels; ++i) {
				create_srv(
					{
						.srv_desc =
							{
								.Format = format,
								.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
								.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
								.Texture2D =
									{
										.MostDetailedMip = i,
										.MipLevels = 1,
									},
							},
					},
					texture.allocation.resource.Get());
			}
		}
		if (texture_desc.usage == TextureUsage::DepthStencil) {
			const DsvCreationDesc dsv_desc = {
				.dsv_desc =
					{
						.Format = ds_format,
						.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
						.Flags = D3D12_DSV_FLAG_NONE,
						.Texture2D =
							{
								.MipSlice = 0,
							},
					},
			};
			texture.dsv_index = create_dsv(dsv_desc, texture.allocation.resource.Get());
		}
		if (texture_desc.usage == TextureUsage::RenderTarget) {
			const RtvCreationDesc rtv_desc = {
				.rtv_desc =
					{
						.Format = format,
						.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
						.Texture2D =
							{
								.MipSlice = 0,
								.PlaneSlice = 0,
							},
					},
			};
			texture.rtv_index = create_rtv(rtv_desc, texture.allocation.resource.Get());
		}
		if (texture_desc.usage != TextureUsage::DepthStencil) {
			if (texture_desc.depthOrArraySize > 1) {
				for (u32 i = 0; i < texture_desc.mipLevels; ++i) {
					const u32 uav_index = create_uav(
						{
							.uav_desc =
								{
									.Format = Texture::get_non_srgb_format(texture_desc.format),
									.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY,
									.Texture2DArray =
										{
											.MipSlice = i,
											.FirstArraySlice = 0,
											.ArraySize = texture_desc.depthOrArraySize,
										},
								},
						},
						texture.allocation.resource.Get());
					if (i == 0) {
						texture.uav_index = uav_index;
					}
				}
			} else {
				for (u32 i = 0; i < texture_desc.mipLevels; ++i) {
					const u32 uav_index = create_uav(
						{
							.uav_desc =
								{
									.Format = Texture::get_non_srgb_format(texture_desc.format),
									.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
									.Texture2D =
										{
											.MipSlice = i,
											.PlaneSlice = 0,
										},
								},
						},
						texture.allocation.resource.Get());
					if (i == 0) {
						texture.uav_index = uav_index;
					}
				}
			}
		}
		// TODO: generate mips m_MipMapGenerator->generate_mips(texture);
		return texture;
	}

	PipelineState GraphicsDevice::create_pipeline_state(const GraphicsPipelineStateCreationDesc& desc) const {
		PROFILE_FUNCTION();
		PipelineState state{m_Device.Get(), desc};
		return state;
	}

	PipelineState GraphicsDevice::create_pipeline_state(const ComputePipelineStateCreationDesc& desc) const {
		PROFILE_FUNCTION();
		PipelineState state(m_Device.Get(), desc);
		return state;
	}

	void GraphicsDevice::init_device_resources() {
		PROFILE_FUNCTION();
		init_directx();
		init_command_queues();
		init_descriptor_heaps();
		init_memory_allocator();
		init_contexts();
		init_bindless_root_signature();
		// TODO: init_mimap_generator();
	}

	void GraphicsDevice::init_swapchain_resources(u32 width, u32 height) {
		PROFILE_FUNCTION();
		// TODO: vsync and check for tearing support
		const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
			.Width = width,
			.Height = height,
			.Format = m_SwapchainBackbufferFormat,
			.Stereo = FALSE,
			.SampleDesc =
				{
					.Count = 1,
					.Quality = 0,
				},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = MAX_FRAMES_IN_FLIGHT,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
		};
		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;
		d3d_check(m_Factory->CreateSwapChainForHwnd(m_DirectCommandQueue->command_queue(), m_WindowHandle, &swapchain_desc, nullptr,
													nullptr, &swapchain));
		d3d_check(swapchain.As(&m_Swapchain));
		m_CurrentFrameId = m_Swapchain->GetCurrentBackBufferIndex();
		create_backbuffer_rtvs();
	}

	void GraphicsDevice::init_directx() {
		PROFILE_FUNCTION();
#if defined(DEBUG) | defined(_DEBUG)
		d3d_check(::D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug)));
		m_Debug->EnableDebugLayer();
		m_Debug->SetEnableGPUBasedValidation(true);
		m_Debug->SetEnableSynchronizedCommandQueueValidation(true);
#endif
		u32 dxgi_factory_creation_flags{};
#if defined(DEBUG) | defined(_DEBUG)
		dxgi_factory_creation_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		d3d_check(::CreateDXGIFactory2(dxgi_factory_creation_flags, IID_PPV_ARGS(&m_Factory)));
		d3d_check(m_Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_Adapter)));
		if (!m_Adapter) {
			CORE_ERROR("Failed to find D3D12 compatible adapter.");
		}
		DXGI_ADAPTER_DESC adapter_desc{};
		d3d_check(m_Adapter->GetDesc(&adapter_desc));
		CORE_INFO("Chosen adapter: {}", WStringToANSI(adapter_desc.Description));
		d3d_check(::D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));
		m_Device->SetName(L"D3D12 Device");
#if defined(DEBUG) | defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_q{};
		d3d_check(m_Device->QueryInterface(IID_PPV_ARGS(&info_q)));
		info_q->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		info_q->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
		info_q->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
		info_q->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, false);
		// Configure queue filter to ignore info message severity.
		std::array<D3D12_MESSAGE_SEVERITY, 1> ignore_message_severities = {
			D3D12_MESSAGE_SEVERITY_INFO,
		};
		// Configure queue filter to ignore individual messages using their ID.
		std::array<D3D12_MESSAGE_ID, 2> ignore_message_ids = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
		};
		D3D12_INFO_QUEUE_FILTER infoQueueFilter = {
			.DenyList =
				{
					.NumSeverities = static_cast<UINT>(ignore_message_severities.size()),
					.pSeverityList = ignore_message_severities.data(),
					.NumIDs = static_cast<UINT>(ignore_message_ids.size()),
					.pIDList = ignore_message_ids.data(),
				},
		};
		d3d_check(info_q->PushStorageFilter(&infoQueueFilter));
		// Debug device represents a graphics device for debugging, while the debug interface controls
		// debug settings and validates pipeline state. Debug device can be used to check for reporting live objects
		// and leaks.
		d3d_check(m_Device->QueryInterface(IID_PPV_ARGS(&m_DebugDevice)));
#endif
	}

	void GraphicsDevice::init_command_queues() {
		PROFILE_FUNCTION();
		m_DirectCommandQueue = stl::make_unique<CommandQueue>(mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT,
															  L"Direct Command Queue");
		m_CopyCommandQueue = stl::make_unique<CommandQueue>(mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_COMMAND_LIST_TYPE_COPY,
															L"Copy Command Queue");
		m_ComputeCommandQueue = stl::make_unique<CommandQueue>(mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE,
															   L"Compute Command Queue");
	}

	void GraphicsDevice::init_descriptor_heaps() {
		PROFILE_FUNCTION();
		m_CbvSrvUavDescriptorHeap = stl::make_unique<DescriptorHeap>(
			mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10000, L"CBV SRV UAV Descriptor Heap");
		m_RtvDescriptorHeap = stl::make_unique<DescriptorHeap>(mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
															   50, L"RTV Descriptor Heap");
		m_DsvDescriptorHeap = stl::make_unique<DescriptorHeap>(mem::ENUM::Engine_Rendering, m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
															   50, L"DSV Descriptor Heap");
		m_SamplerDescriptorHeap = stl::make_unique<DescriptorHeap>(mem::ENUM::Engine_Rendering, m_Device.Get(),
																   D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, L"Sampler Descriptor Heap");
	}

	void GraphicsDevice::init_memory_allocator() {
		PROFILE_FUNCTION();
		m_MemoryAllocator = stl::make_unique<MemoryAllocator>(mem::ENUM::Engine_Rendering, m_Device.Get(), m_Adapter.Get());
	}

	void GraphicsDevice::init_contexts() {
		PROFILE_FUNCTION();
		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			m_PerFrameGraphicsContexts[i] = stl::make_unique<GraphicsContext>(mem::ENUM::Engine_Rendering, this);
		}
		m_CopyContext = stl::make_unique<CopyContext>(mem::ENUM::Engine_Rendering, this);
		m_ComputeContextQueue.push(stl::make_unique<ComputeContext>(mem::ENUM::Engine_Rendering, this));
	}

	void GraphicsDevice::init_bindless_root_signature() {
		PROFILE_FUNCTION();
		PipelineState::create_bindless_root_signature(m_Device.Get(), L"bindless.hlsl");
	}

	void GraphicsDevice::create_backbuffer_rtvs() {
		PROFILE_FUNCTION();
		DescriptorHandle rtv_handle = m_RtvDescriptorHeap->descriptor_handle_from_start();
		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer{};
			d3d_check(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)));
			m_Device->CreateRenderTargetView(backbuffer.Get(), nullptr, rtv_handle.cpu_descriptor_handle);
			m_BackBuffers[i].allocation.resource = backbuffer;
			m_BackBuffers[i].allocation.resource->SetName(L"Swapchain Backbuffer");
			m_BackBuffers[i].rtv_index = m_RtvDescriptorHeap->descriptor_index(rtv_handle);
			m_RtvDescriptorHeap->offset_descriptor(rtv_handle);
		}
		if (!m_IsInitialized) {
			m_RtvDescriptorHeap->offset_current_handle(MAX_FRAMES_IN_FLIGHT);
		}
	}

	u32 GraphicsDevice::create_cbv(const CbvCreationDesc& desc) const {
		PROFILE_FUNCTION();
		const u32 cbv_index = m_CbvSrvUavDescriptorHeap->current_descriptor_index();
		m_Device->CreateConstantBufferView(&desc.cbv_desc, m_CbvSrvUavDescriptorHeap->current_descriptor_handle().cpu_descriptor_handle);
		m_CbvSrvUavDescriptorHeap->offset_current_handle();
		return cbv_index;
	}

	u32 GraphicsDevice::create_srv(const SrvCreationDesc& desc, ID3D12Resource* resource) const {
		PROFILE_FUNCTION();
		const u32 srv_index = m_CbvSrvUavDescriptorHeap->current_descriptor_index();
		m_Device->CreateShaderResourceView(resource, &desc.srv_desc,
										   m_CbvSrvUavDescriptorHeap->current_descriptor_handle().cpu_descriptor_handle);
		m_CbvSrvUavDescriptorHeap->offset_current_handle();
		return srv_index;
	}

	u32 GraphicsDevice::create_uav(const UavCreationDesc& desc, ID3D12Resource* resource) const {
		PROFILE_FUNCTION();
		const u32 uav_index = m_CbvSrvUavDescriptorHeap->current_descriptor_index();
		m_Device->CreateUnorderedAccessView(resource, nullptr, &desc.uav_desc,
											m_CbvSrvUavDescriptorHeap->current_descriptor_handle().cpu_descriptor_handle);
		m_CbvSrvUavDescriptorHeap->offset_current_handle();
		return uav_index;
	}

	u32 GraphicsDevice::create_rtv(const RtvCreationDesc& desc, ID3D12Resource* resource) const {
		PROFILE_FUNCTION();
		const u32 rtv_index = m_RtvDescriptorHeap->current_descriptor_index();
		m_Device->CreateRenderTargetView(resource, &desc.rtv_desc, m_RtvDescriptorHeap->current_descriptor_handle().cpu_descriptor_handle);
		m_RtvDescriptorHeap->offset_current_handle();
		return rtv_index;
	}

	u32 GraphicsDevice::create_dsv(const DsvCreationDesc& desc, ID3D12Resource* resource) const {
		PROFILE_FUNCTION();
		const u32 dsv_index = m_DsvDescriptorHeap->current_descriptor_index();
		m_Device->CreateDepthStencilView(resource, &desc.dsv_desc, m_DsvDescriptorHeap->current_descriptor_handle().cpu_descriptor_handle);
		m_DsvDescriptorHeap->offset_current_handle();
		return dsv_index;
	}
} // namespace Sapfire::d3d
