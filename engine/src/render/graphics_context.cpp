#include "engpch.h"

#include "d3d12.h"
#include <combaseapi.h>
#include "render/context.h"
#include "render/descriptor_heap.h"
#include "render/graphics_context.h"
#include "render/graphics_device.h"
#include "render/pipeline_state.h"

namespace Sapfire::d3d {

	GraphicsContext::GraphicsContext(GraphicsDevice* device) : m_GraphicsDevice(*device) {
		d3d_check(device->device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
		d3d_check(device->device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr,
													  IID_PPV_ARGS(&m_CommandList)));
		set_descriptor_heaps();
		d3d_check(m_CommandList->Close());
	}

	void GraphicsContext::reset_context() {
		Context::reset_context();
		set_descriptor_heaps();
	}

	void GraphicsContext::clear_render_target_view(const Texture& render_target, const stl::span<const f32, 4> color) {
		const auto rtv_descriptor_handle = m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(render_target.rtv_index);
		m_CommandList->ClearRenderTargetView(rtv_descriptor_handle.cpu_descriptor_handle, color.data(), 0, nullptr);
	}

	void GraphicsContext::clear_render_target_view(const stl::span<Texture> render_targets, const stl::span<const f32, 4> color) {
		for (auto& target : render_targets) {
			const auto rtv_descriptor_handle = m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(target.rtv_index);
			m_CommandList->ClearRenderTargetView(rtv_descriptor_handle.cpu_descriptor_handle, color.data(), 0, nullptr);
		}
	}

	void GraphicsContext::clear_render_target_view(const Texture& texture, f32 color) {
		const auto rtv_descriptor_handle = m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(texture.rtv_index);
		m_CommandList->ClearRenderTargetView(rtv_descriptor_handle.cpu_descriptor_handle, (FLOAT*)&color, 0, nullptr);
	}

	void GraphicsContext::clear_depth_stencil_view(const Texture& texture) {
		const auto dsv_descriptor_handle = m_GraphicsDevice.dsv_descriptor_heap()->descriptor_handle_from_index(texture.dsv_index);
		m_CommandList->ClearDepthStencilView(dsv_descriptor_handle.cpu_descriptor_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);
	}
	void GraphicsContext::set_descriptor_heaps() const {
		const stl::array<const DescriptorHeap*, 2> shader_visible_descriptor_heaps{m_GraphicsDevice.cbv_srv_uav_descriptor_heap(),
																				   m_GraphicsDevice.sampler_descriptor_heap()};
		stl::array<ID3D12DescriptorHeap*, 2> descriptor_heaps{};
		for (size_t i = 0; i < shader_visible_descriptor_heaps.size(); ++i) {
			descriptor_heaps[i] = shader_visible_descriptor_heaps[i]->descriptor_heap();
		}
		m_CommandList->SetDescriptorHeaps(static_cast<UINT>(descriptor_heaps.size()), descriptor_heaps.data());
	}

	// pipeline configuration
	void GraphicsContext::set_graphics_pipeline_state(const PipelineState& state) const { m_CommandList->SetPipelineState(state.pso()); }

	void GraphicsContext::set_graphics_root_signature() const { m_CommandList->SetGraphicsRootSignature(PipelineState::root_signature()); }

	void GraphicsContext::set_graphics_root_signature_and_pipeline(const PipelineState& state) const {
		m_CommandList->SetGraphicsRootSignature(PipelineState::root_signature());
		m_CommandList->SetPipelineState(state.pso());
	}

	void GraphicsContext::set_index_buffer(const Buffer& buffer) const {
		const D3D12_INDEX_BUFFER_VIEW index_buffer_view{
			.BufferLocation = buffer.allocation.resource->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(buffer.size_in_bytes),
			.Format = DXGI_FORMAT_R16_UINT,
		};
		m_CommandList->IASetIndexBuffer(&index_buffer_view);
	}

	void GraphicsContext::set_32_bit_graphics_constants(const void* render_resources) const {
		m_CommandList->SetGraphicsRoot32BitConstants(0, NUMBER_32_BIT_CONSTANTS, render_resources, 0);
	}

	void GraphicsContext::set_viewport(const D3D12_VIEWPORT& viewport) const {
		static D3D12_RECT scissor{
			.left = 0,
			.top = 0,
			.right = LONG_MAX,
			.bottom = LONG_MAX,
		};
		m_CommandList->RSSetViewports(1, &viewport);
		m_CommandList->RSSetScissorRects(1, &scissor);
	}

	void GraphicsContext::set_primitive_topology_layout(const D3D_PRIMITIVE_TOPOLOGY topology) const {
		m_CommandList->IASetPrimitiveTopology(topology);
	}

	void GraphicsContext::set_render_target(const Texture& target) const {
		const auto rtv_descriptor_handle = m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(target.rtv_index);
		m_CommandList->OMSetRenderTargets(1, &rtv_descriptor_handle.cpu_descriptor_handle, FALSE, nullptr);
	}

	void GraphicsContext::set_render_target(const Texture& target, const Texture& depth_stencil_texture) const {
		const auto rtv_descriptor_handle = m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(target.rtv_index);
		const auto dsv_descriptor_handle =
			m_GraphicsDevice.dsv_descriptor_heap()->descriptor_handle_from_index(depth_stencil_texture.dsv_index);
		m_CommandList->OMSetRenderTargets(1, &rtv_descriptor_handle.cpu_descriptor_handle, FALSE,
										  &dsv_descriptor_handle.cpu_descriptor_handle);
	}

	void GraphicsContext::set_render_target(stl::span<Texture> targets, const Texture& depth_stencil_texture) const {
		stl::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv_handles{targets.size()};
		for (auto& target : targets) {
			rtv_handles.push_back(
				m_GraphicsDevice.rtv_descriptor_heap()->descriptor_handle_from_index(target.rtv_index).cpu_descriptor_handle);
		}
		const auto dsv_descriptor_handle =
			m_GraphicsDevice.dsv_descriptor_heap()->descriptor_handle_from_index(depth_stencil_texture.dsv_index);
		m_CommandList->OMSetRenderTargets(static_cast<u32>(rtv_handles.size()), rtv_handles.data(), TRUE,
										  &dsv_descriptor_handle.cpu_descriptor_handle);
	}

	void GraphicsContext::copy_resource(ID3D12Resource* source, ID3D12Resource* destination) const {
		m_CommandList->CopyResource(destination, source);
	}

	void GraphicsContext::draw_instance_indexed(u32 index_count, u32 instance_count /*= 1*/) const {
		m_CommandList->DrawIndexedInstanced(index_count, instance_count, 0, 0, 0);
	}

	void GraphicsContext::draw_indexed(u32 index_count, u32 instance_count /*= 1*/) const {
		m_CommandList->DrawInstanced(index_count, instance_count, 0, 0);
	}

} // namespace Sapfire::d3d
