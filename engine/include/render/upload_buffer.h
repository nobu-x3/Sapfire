#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "d3dx12_core.h"
#include "render/d3d_util.h"

namespace Sapfire::d3d {
	template <typename T>
	class UploadBuffer {

	public:
		UploadBuffer(ID3D12Device* device, u32 element_count, bool is_constant_buffer, const std::wstring& name = L"") :
			m_IsConstantBuffer(is_constant_buffer) {
			m_ElementByteSize = sizeof(T);
			if (is_constant_buffer) {
				m_ElementByteSize = calculate_constant_buffer_byte_size(m_ElementByteSize);
			}
			auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * element_count);
			d3d_check(device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
													  nullptr, IID_PPV_ARGS(&m_UploadBuffer)));
			if (!name.empty()) {
				m_UploadBuffer->SetName(name.c_str());
			}
			d3d_check(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
		}

		~UploadBuffer() {
			if (m_UploadBuffer != nullptr) {
				m_UploadBuffer->Unmap(0, nullptr);
			}
			m_MappedData = nullptr;
		}

		UploadBuffer(const UploadBuffer& other) = delete;
		UploadBuffer& operator=(const UploadBuffer& other) = delete;

		inline ID3D12Resource* resource() const { return m_UploadBuffer.Get(); }

		inline void copy_data(int element_index, const T& data) {
			memcpy(&m_MappedData[element_index * m_ElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		BYTE* m_MappedData = nullptr;
		int m_ElementByteSize = 0;
		bool m_IsConstantBuffer = false;
	};
} // namespace Sapfire::d3d
