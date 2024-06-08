#include "engpch.h"

#include "d3dx12_barriers.h"
#include "render/context.h"
#include "render/d3d_util.h"

namespace Sapfire::d3d {
	using namespace Microsoft::WRL;
	void transition_resource(const ComPtr<ID3D12GraphicsCommandList>& command_list, const ComPtr<ID3D12Resource>& resource,
							 D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), before, after);
		command_list->ResourceBarrier(1, &barrier);
	}

	void Context::add_resource_barrier(ID3D12Resource* resource, const D3D12_RESOURCE_STATES previous_states,
									   const D3D12_RESOURCE_STATES new_states) {
		m_ResourceBarriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource, previous_states, new_states));
	}

	void Context::add_resource_barrier(ID3D12Resource* resource) {
		m_ResourceBarriers.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource));
	}

	void Context::add_resource_barrier(const CD3DX12_RESOURCE_BARRIER& resource_barrier) {
		m_ResourceBarriers.emplace_back(resource_barrier);
	}

	void Context::execute_resource_barriers() {
		m_CommandList->ResourceBarrier(static_cast<u32>(m_ResourceBarriers.size()), m_ResourceBarriers.data());
		m_ResourceBarriers.clear();
	}

	void Context::reset_context() {
		d3d_check(m_CommandAllocator->Reset());
		d3d_check(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));
	}
} // namespace Sapfire::d3d
