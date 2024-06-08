#pragma once

#include "core/core.h"
namespace Sapfire::d3d{

    // Holds CPU and GPU descriptor handles
    struct SFAPI DescriptorHandle {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle{};
        u32 descriptor_size{};

        inline void offset() {
            cpu_descriptor_handle.ptr += descriptor_size;
            gpu_descriptor_handle.ptr += descriptor_size;
        }
    };

    // Continuous linear allocation which stores descriptors
    class SFAPI DescriptorHeap {
        public:
            explicit DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, u32 descriptor_count, stl::wstring_view heap_name);
            ~DescriptorHeap() = default;
            DescriptorHeap(const DescriptorHeap&) = delete;
            DescriptorHeap& operator=(const DescriptorHeap&) = delete;
            DescriptorHeap(DescriptorHeap&&) = delete;
            DescriptorHeap& operator=(DescriptorHeap&&) = delete;

            inline ID3D12DescriptorHeap* descriptor_heap() const {
                return m_DescriptorHeap.Get();
            }

            inline u32 descriptor_size() const { return m_DescriptorSize;}

            inline DescriptorHandle descriptor_handle_from_start() const {
                return m_DescriptorHandleFromHeapStart;
            }

            inline DescriptorHandle current_descriptor_handle() const {
                return m_CurrentDescriptorHandle;
            }

            DescriptorHandle descriptor_handle_from_index(u32 index) const;

            [[nodiscard]] u32 descriptor_index(const DescriptorHandle& handle) const;
            [[nodiscard]] u32 current_descriptor_index() const;

            void offset_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE& handle, u32 offset = 1) const;
            void offset_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE& handle, u32 offset = 1) const;
            void offset_descriptor(DescriptorHandle& handle, u32 offset = 1) const;
            void offset_current_handle(u32 offset = 1);

        private:
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap{};
            u32 m_DescriptorSize{};
            DescriptorHandle m_DescriptorHandleFromHeapStart{};
            DescriptorHandle m_CurrentDescriptorHandle{};

    };
}
