#pragma once

#include <wrl.h>

namespace Sapfire::d3d {
    class Context;

	class CommandQueue {
	public:
		explicit CommandQueue(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE cmd_list_type, const stl::wstring_view name);
		~CommandQueue() = default;
		CommandQueue(const CommandQueue&) = delete;
		CommandQueue& operator=(const CommandQueue&) = delete;
		CommandQueue(CommandQueue&&) = delete;
		CommandQueue& operator=(CommandQueue&&) = delete;

		inline ID3D12CommandQueue* command_queue() const { return m_CommandQueue.Get(); }

		inline u64 current_completed_fence_value() const { return m_Fence->GetCompletedValue(); }

        // The user / engine must take care that a context passed into the execute function can be used in the future
        // (i.e proper synchronization must occur).
        // For now, each context consist of a command list and command allocator, and there is one command list per
        // frame in flight (for direct command queue).
        void execute_context(const stl::span<const Context*> contexts);

        bool is_fence_complete(u64 fence_vaule) const;
        u64 signal();
        void wait_for_fence_value(u64 fence_value) const;
        void flush();

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue{};
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence{};
		u64 m_FenceValue{0};
	};
} // namespace Sapfire::d3d
