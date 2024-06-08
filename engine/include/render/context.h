#pragma once

#include <wrl.h>
#include "d3d12.h"
#include "d3dx12_barriers.h"
namespace Sapfire::d3d {
	void SFAPI transition_resource(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& command_list,
								   const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES before,
								   D3D12_RESOURCE_STATES after);

	constexpr u32 NUMBER_32_BIT_CONSTANTS = 64;

    // Wrapper for command list and command allocator. Batches resource barriers. Can execute commands of any type.
    class SFAPI Context {
        public:
            explicit Context() = default;
            virtual ~Context() = default;

            inline ID3D12GraphicsCommandList1*  command_list() const {
                return m_CommandList.Get();
            }

            void add_resource_barrier(ID3D12Resource* resource, const D3D12_RESOURCE_STATES previous_states, const D3D12_RESOURCE_STATES new_states);

            void add_resource_barrier(ID3D12Resource* resource);

            void add_resource_barrier(const CD3DX12_RESOURCE_BARRIER& resource_barrier);

            void execute_resource_barriers();

            virtual void reset_context();

        protected:
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> m_CommandList{};
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator{};

            // execute_resource_barriers() must be invoked before command list is sent over to device for execution and batched as much as possible
            stl::vector<CD3DX12_RESOURCE_BARRIER> m_ResourceBarriers;
    };
} // namespace Sapfire::d3d
