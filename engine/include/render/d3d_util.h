#pragma once
#include "d3dcommon.h"
#include "core/core.h"
#if defined(SF_PLATFORM_WINDOWS)
#include <D3Dcompiler.h>
#include <comdef.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <winnt.h>
#include <wrl.h>
#include <wrl/client.h>

namespace Sapfire::d3d {

	constexpr int MAX_FRAMES_IN_FLIGHT = 3;

	inline std::wstring AnsiToWString(const std::string& str) {
		WCHAR buffer[512];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
		return std::wstring(buffer);
	}

	inline std::string WStringToANSI(const std::wstring_view inputWString) {
		std::string result{};
		const std::wstring input{inputWString};
		const int32_t length = ::WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, NULL, 0, NULL, NULL);
		if (length > 0) {
			result.resize(size_t(length) - 1);
			WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, result.data(), length, NULL, NULL);
		}
		return result;
	}

	class DxException {
	public:
		SFAPI DxException() = default;
		SFAPI DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
			ErrorCode(hr), FunctionName(functionName), Filename(filename), LineNumber(lineNumber) {}

		std::wstring SFAPI to_string() const {
			// Get the string description of the error code.
			_com_error err(ErrorCode);
#ifdef UNICODE
            std::wstring msg = err.ErrorMessage();
            return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
#else
            std::wstring msg = AnsiToWString(err.ErrorMessage());
            return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
#endif
		}

		HRESULT ErrorCode = S_OK;
		std::wstring FunctionName;
		std::wstring Filename;
		int LineNumber = -1;
	};
#ifndef d3d_check
#define d3d_check(x)                                                                                                                       \
	{                                                                                                                                      \
		HRESULT hr__ = (x);                                                                                                                \
		std::wstring wfn = d3d::AnsiToWString(__FILE__);                                                                                   \
		if (FAILED(hr__)) {                                                                                                                \
			throw d3d::DxException(hr__, L#x, wfn, __LINE__);                                                                              \
		}                                                                                                                                  \
	}
#endif

	Microsoft::WRL::ComPtr<ID3D12Resource> SFAPI create_default_buffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmd_list,
																	   const void* init_data, u32 size,
																	   Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer);

	inline u32 calculate_constant_buffer_byte_size(u32 byte_size) { return (byte_size + 255) & ~255; }

	struct SubmeshGeometry {
		u32 index_count = 0;
		u32 start_index_location = 0;
		s32 base_vertex_location = 0;
	};

	struct MeshGeometry {
		// Give it a name so we can look it up by name.
		std::string name;

		// System memory copies.  Use Blobs because the vertex/index format can
		// be generic. It is up to the client to cast appropriately.
		Microsoft::WRL::ComPtr<ID3DBlob> vertex_buffer_cpu = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> index_buffer_cpu = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_gpu = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_gpu = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_uploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_uploader = nullptr;

		// Data about the buffers.
		u32 vertex_byte_stride = 0;
		u32 vertex_buffer_byte_size = 0;
		DXGI_FORMAT index_format = DXGI_FORMAT_R16_UINT;
		u32 index_buffer_byte_size = 0;

		// A MeshGeometry may store multiple geometries in one vertex/index
		// buffer. Use this container to define the Submesh geometries so we can
		// draw the Submeshes individually.
		std::unordered_map<std::string, SubmeshGeometry> draw_args;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const {
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
			vbv.StrideInBytes = vertex_byte_stride;
			vbv.SizeInBytes = vertex_buffer_byte_size;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const {
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
			ibv.Format = index_format;
			ibv.SizeInBytes = index_buffer_byte_size;

			return ibv;
		}

		// We can free this memory after we finish upload to the GPU.
		void DisposeUploaders() {
			vertex_buffer_uploader = nullptr;
			index_buffer_uploader = nullptr;
		}
	};
} // namespace Sapfire::d3d
#endif
