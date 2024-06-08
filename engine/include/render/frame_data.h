#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include "core/core.h"
#include "d3d12.h"
#include "math/math_helper.h"
#include "render/lights.h"
#include "render/material.h"
#include "render/upload_buffer.h"

namespace Sapfire::d3d {

	struct Texture {
		// Unique material name for lookup.
		std::string Name;
		std::wstring Filename;
		Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
	};

	struct PerDrawConstants {
		u32 position_buffer_idx = 0;
		u32 normal_buffer_idx = 0;
		u32 tangent_buffer_idx = 0;
		u32 uv_buffer_idx = 0;
		u32 scene_cbuffer_idx = 0;
		u32 pass_cbuffer_idx = 0;
		u32 material_cbuffer_idx = 0;
        u32 texture_cbuffer_idx = 0;
	};

	struct ObjectConstants {
		DirectX::XMFLOAT4X4 World = math::Identity4x4();
	};

	struct PassConstants {
		DirectX::XMFLOAT4X4 view = math::Identity4x4();
		DirectX::XMFLOAT4X4 inv_view = math::Identity4x4();
		DirectX::XMFLOAT4X4 proj = math::Identity4x4();
		DirectX::XMFLOAT4X4 inv_proj = math::Identity4x4();
		DirectX::XMFLOAT4X4 view_proj = math::Identity4x4();
		DirectX::XMFLOAT4X4 inv_view_proj = math::Identity4x4();
		DirectX::XMFLOAT3 EyePosW = {0.0f, 0.0f, 0.0f};
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 render_target_size = {0.0f, 0.0f};
		DirectX::XMFLOAT2 inv_render_target_size = {0.0f, 0.0f};
		float near_z = 0.0f;
		float far_z = 0.0f;
		float total_time = 0.0f;
		float delta_time = 0.0f;
		DirectX::XMFLOAT4 ambient_light = {0.0f, 0.0f, 0.0f, 1.0f};
		// Indices [0, NUM_DIR_LIGHTS) are directional lights;
		// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
		// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
		// are spot lights for a maximum of MaxLights per object.
		Light Lights[MaxLights];
	};

	struct FrameData {
		FrameData(ID3D12Device* device, u32 pass_count, u32 object_count, u32 material_count, u32 frame_number);
		FrameData(const FrameData&) = delete;
		FrameData& operator=(const FrameData&) = delete;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmd_list_allocator;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heaps_cbv_uav_srv;
		stl::unique_ptr<UploadBuffer<ObjectConstants>> object_cb = nullptr;
		stl::unique_ptr<UploadBuffer<PassConstants>> pass_cb = nullptr;
		stl::unique_ptr<UploadBuffer<MaterialConstants>> materials_cb = nullptr;
		u64 fence = 0;

		void init_object_cb();
		void init_pass_cb();
		void init_materials_cb();

	private:
		ID3D12Device* m_Device = nullptr;
		u32 m_PassCount = 0;
		u32 m_ObjectCount = 0;
		u32 m_MaterialCount = 0;
		u32 m_FrameNumber = 0;
	};

} // namespace Sapfire::d3d
