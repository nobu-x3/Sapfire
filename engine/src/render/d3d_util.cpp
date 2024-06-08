#include "engpch.h"

#include "render/d3d_util.h"

namespace Sapfire::d3d {
	Microsoft::WRL::ComPtr<ID3D12Resource> create_default_buffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmd_list,
																 const void* init_data, u32 size,
																 Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer) {
		Microsoft::WRL::ComPtr<ID3D12Resource> default_buffer;
		auto res_desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		auto default_heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		d3d_check(device->CreateCommittedResource(&default_heap_props, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_COMMON,
												  nullptr, IID_PPV_ARGS(default_buffer.GetAddressOf())));
		auto upload_heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		d3d_check(device->CreateCommittedResource(&upload_heap_props, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
												  nullptr, IID_PPV_ARGS(upload_buffer.GetAddressOf())));
		D3D12_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pData = init_data;
		subresource_data.RowPitch = size;
		subresource_data.SlicePitch = size;
		auto transition_copy_dest =
			CD3DX12_RESOURCE_BARRIER::Transition(default_buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmd_list->ResourceBarrier(1, &transition_copy_dest);
		UpdateSubresources<1>(cmd_list, default_buffer.Get(), upload_buffer.Get(), 0, 0, 1, &subresource_data);
		auto transition_generic_read =
			CD3DX12_RESOURCE_BARRIER::Transition(default_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmd_list->ResourceBarrier(1, &transition_generic_read);
		return default_buffer;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> compile_shader(const std::filesystem::path& filename, const D3D_SHADER_MACRO* defines,
													stl::string_view entrypoint, stl::string_view target) {
		u32 compile_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		HRESULT hr = S_OK;
		Microsoft::WRL::ComPtr<ID3DBlob> byte_code = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errors = nullptr;
		hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.data(), target.data(),
								compile_flags, 0, &byte_code, &errors);
		if (errors != nullptr) {
			::OutputDebugStringA((char*)errors->GetBufferPointer());
		}
		d3d_check(hr);
		return byte_code;
	}
} // namespace Sapfire::d3d
