#pragma once

#include <wrl/client.h>
#include "D3D12MemAlloc.h"
#include "core/core.h"
#include "d3d12.h"
#include "dxgiformat.h"

namespace Sapfire::d3d {

	constexpr uint32_t INVALID_INDEX_U32 = 0xFFFFFFFF;
	struct SFAPI CbvCreationDesc {
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
	};

	struct SFAPI DsvCreationDesc {
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	};

	struct SFAPI RtvCreationDesc {
		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
	};

	struct SFAPI UavCreationDesc {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
	};

	struct SFAPI SrvCreationDesc {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	};

	// If texture data is uploaded elsewhere, use TextureFromData and fully initialize TextureCreationDesc.
	enum class TextureUsage { DepthStencil, RenderTarget, TextureFromPath, TextureFromData, HDRTextureFromPath, CubeMap, UAVTexture };

	struct SFAPI TextureCreationDesc {
		TextureUsage usage{};
		u32 width{};
		u32 height{};
		DXGI_FORMAT format{DXGI_FORMAT_R8G8B8A8_UNORM};
        stl::optional<D3D12_RESOURCE_STATES> optional_initial_state{D3D12_RESOURCE_STATE_COMMON};
		u32 mipLevels{1};
		u32 depthOrArraySize{1};
		u32 bytesPerPixel{4};
		stl::wstring_view name{};
		stl::wstring path{};
	};

	// Buffer related functions / enum's.
	// Vertex buffer's are not used in the engine. Rather vertex pulling is used and data is stored in struct SFAPIured
	// buffer.
	enum class BufferUsage {
		UploadBuffer,
		IndexBuffer,
		StructuredBuffer,
		ConstantBuffer,
	};

	struct SFAPI BufferCreationDesc {
		BufferUsage usage{};
		stl::wstring_view name{};
	};

	struct SFAPI ResourceCreationDesc {
		D3D12_RESOURCE_DESC resource_desc{};

		[[nodiscard]] inline static ResourceCreationDesc create_buffer_resource_creation_desc(u64 size) {
			return ResourceCreationDesc{
				.resource_desc =
					{
						.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
						.Width = size,
						.Height = 1,
						.DepthOrArraySize = 1,
						.MipLevels = 1,
						.Format = DXGI_FORMAT_UNKNOWN,
						.SampleDesc{.Count = 1, .Quality = 0},
						.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
						.Flags = D3D12_RESOURCE_FLAG_NONE,
					},
			};
		}
	};

	// Struct's related to pipeline's.
	struct SFAPI ShaderModule {
		stl::wstring_view vertexShaderPath{};
		stl::wstring_view vertexEntryPoint{L"VS"};

		stl::wstring_view pixelShaderPath{};
		stl::wstring_view pixelEntryPoint{L"PS"};

		stl::wstring_view computeShaderPath{};
		stl::wstring_view computeEntryPoint{L"CS"};
	};

	enum class ShaderType : u8 { Vertex, Pixel, Compute, RootSignature };

	enum class FrontFaceWindingOrder { Clockwise, CounterClockwise };

	struct SFAPI GraphicsPipelineStateCreationDesc {
		ShaderModule shader_module{};
		stl::vector<DXGI_FORMAT> rtv_formats{DXGI_FORMAT_R16G16B16A16_FLOAT};
		u32 rtv_count{1};
		DXGI_FORMAT depth_format{DXGI_FORMAT_D32_FLOAT};
		D3D12_COMPARISON_FUNC depth_comparison_func{D3D12_COMPARISON_FUNC_LESS};
		FrontFaceWindingOrder front_face_winding_order{FrontFaceWindingOrder::Clockwise};
		D3D12_CULL_MODE cull_mode{D3D12_CULL_MODE_BACK};
		stl::wstring_view pipeline_name{};
	};

	struct SFAPI ComputePipelineStateCreationDesc {
		stl::wstring_view shader_path{};
		stl::wstring_view pipeline_name{};
	};

	struct SFAPI Allocation {
		Allocation() = default;
		Allocation(const Allocation&);
		Allocation& operator=(const Allocation&);
		Allocation(Allocation&&) noexcept;
		Allocation& operator=(Allocation&&) noexcept;

		void update(const void* data, size_t size);
		void reset();

		Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation{};
		stl::optional<void*> mapped_data{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource{};
	};

	struct SFAPI Buffer {
		// To be used primarily for constant buffers.
		void update(const void* data);

		Allocation allocation{};
		size_t size_in_bytes{};

		uint32_t srv_index{INVALID_INDEX_U32};
		uint32_t uav_index{INVALID_INDEX_U32};
		uint32_t cbv_index{INVALID_INDEX_U32};
	};

	struct SFAPI Texture {
		u32 width{};
		u32 height{};
		Allocation allocation{};
		u32 srv_index{INVALID_INDEX_U32};
		u32 uav_index{INVALID_INDEX_U32};
		u32 dsv_index{INVALID_INDEX_U32};
		u32 rtv_index{INVALID_INDEX_U32};

		static bool is_srgb(DXGI_FORMAT format);
		static DXGI_FORMAT get_non_srgb_format(DXGI_FORMAT format);
	};
} // namespace Sapfire::d3d
