#include "d3d12.h"
#include "engpch.h"

#include "render/compute_context.h"
#include "render/context.h"
#include "render/d3d_util.h"
#include "render/descriptor_heap.h"
#include "render/graphics_device.h"
#include "render/pipeline_state.h"

namespace Sapfire::d3d {

	ComputeContext::ComputeContext(GraphicsDevice* device) : m_GraphicsDevice(*device) {
		d3d_check(device->device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_CommandAllocator)));
		d3d_check(device->device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_CommandAllocator.Get(), nullptr,
													  IID_PPV_ARGS(&m_CommandList)));
		stl::array<const DescriptorHeap*, 2> descriptor_heaps = {device->cbv_srv_uav_descriptor_heap(), device->sampler_descriptor_heap()};
		set_descriptor_heaps(descriptor_heaps);
		d3d_check(m_CommandList->Close());
	}

	void ComputeContext::reset_context() {
        PROFILE_FUNCTION();
		Context::reset_context();
		stl::array<const DescriptorHeap*, 2> descriptor_heaps = {m_GraphicsDevice.cbv_srv_uav_descriptor_heap(),
																 m_GraphicsDevice.sampler_descriptor_heap()};
		set_descriptor_heaps(descriptor_heaps);
	}

	void ComputeContext::dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z) const {
		m_CommandList->Dispatch(thread_group_x, thread_group_y, thread_group_z);
	}

	void ComputeContext::set_descriptor_heaps(stl::span<const DescriptorHeap*> shader_visible_descriptor_heaps) const {
        PROFILE_FUNCTION();
		stl::vector<ID3D12DescriptorHeap*> descriptor_heaps{shader_visible_descriptor_heaps.size()};
		for (u32 i = 0; i < shader_visible_descriptor_heaps.size(); ++i) {
			descriptor_heaps[i] = shader_visible_descriptor_heaps[i]->descriptor_heap();
		}
		m_CommandList->SetDescriptorHeaps(static_cast<UINT>(descriptor_heaps.size()), descriptor_heaps.data());
	}

	void ComputeContext::set_compute_root_signature_and_pipeline(const PipelineState& state) const {
		m_CommandList->SetComputeRootSignature(state.root_signature());
		m_CommandList->SetPipelineState(state.pso());
	}

	void ComputeContext::set_32bit_compute_constants(const void* render_resources) const {
		m_CommandList->SetComputeRoot32BitConstants(0, NUMBER_32_BIT_CONSTANTS, render_resources, 0);
	}
} // namespace Sapfire::d3d
