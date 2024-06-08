#include "engpch.h"

#include "d3d12.h"
#include "render/command_queue.h"
#include "render/context.h"
#include "render/d3d_util.h"

namespace Sapfire::d3d {

	CommandQueue::CommandQueue(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE cmd_list_type, const stl::wstring_view name) {
		const D3D12_COMMAND_QUEUE_DESC command_queue_desc{
			.Type = cmd_list_type,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0u,
		};
		d3d_check(device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&m_CommandQueue)));
		m_CommandQueue->SetName(name.data());
		// CPU & GPU sync fence
		d3d_check(device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
		m_Fence->SetName(name.data());
	}

	void CommandQueue::execute_context(const stl::span<const Context*> contexts) {
		stl::vector<ID3D12CommandList*> command_lists{contexts.size()};
		for (u32 i = 0; i < contexts.size(); ++i) {
			d3d_check(contexts[i]->command_list()->Close());
			command_lists[i] = static_cast<ID3D12CommandList*>(contexts[i]->command_list());
		}
		m_CommandQueue->ExecuteCommandLists(static_cast<UINT>(command_lists.size()), command_lists.data());
	}

	bool CommandQueue::is_fence_complete(u64 fence_value) const { return m_Fence->GetCompletedValue() >= fence_value; }

	u64 CommandQueue::signal() {
		m_FenceValue++;
		d3d_check(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue));
		return m_FenceValue;
	}

	void CommandQueue::wait_for_fence_value(u64 fence_value) const {
		if (!is_fence_complete(fence_value)) {
			d3d_check(m_Fence->SetEventOnCompletion(fence_value, nullptr));
		}
	}

	void CommandQueue::flush() {
		const u64 fence_value_to_wait_for = signal();
		wait_for_fence_value(fence_value_to_wait_for);
	}
} // namespace Sapfire::d3d
