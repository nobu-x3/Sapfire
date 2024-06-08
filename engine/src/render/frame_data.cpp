#include "engpch.h"

#include <string>
#include "core/core.h"
#include "d3d12.h"
#include "render/frame_data.h"
#include "render/material.h"
#include "render/upload_buffer.h"

namespace Sapfire::d3d {

	FrameData::FrameData(ID3D12Device* device, u32 pass_count, u32 object_count, u32 material_count, u32 frame_number) :
		m_Device(device), m_PassCount(pass_count), m_ObjectCount(object_count), m_MaterialCount(material_count),
		m_FrameNumber(frame_number) {
		d3d_check(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_list_allocator.GetAddressOf())));
		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{};
		descriptor_heap_desc.NumDescriptors = 512;
		descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		d3d_check(device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heaps_cbv_uav_srv)));
		descriptor_heaps_cbv_uav_srv->SetName(L"CBV SRV UAV Descriptor Heap");
	}

	void FrameData::init_object_cb() {
		object_cb = stl::make_unique<UploadBuffer<ObjectConstants>>(m_Device, m_ObjectCount, true,
																	L"Object Constants " + std::to_wstring(m_FrameNumber));
	}

	void FrameData::init_pass_cb() {
		pass_cb =
			stl::make_unique<UploadBuffer<PassConstants>>(m_Device, m_PassCount, true, L"Pass Constants " + std::to_wstring(m_FrameNumber));
	}

	void FrameData::init_materials_cb() {
		materials_cb = stl::make_unique<UploadBuffer<MaterialConstants>>(m_Device, m_MaterialCount, true,
																		 L"Material Constants " + std::to_wstring(m_FrameNumber));
	}
} // namespace Sapfire::d3d
