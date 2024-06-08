#pragma once

#include <DirectXMath.h>
#include "core/core.h"
#include "render/d3d_util.h"
#include "render/resources.h"

namespace Sapfire::d3d {

	struct SFAPI MaterialConstants {
		DirectX::XMFLOAT4 diffuse_albedo{1.f, 1.f, 1.f, 1.f};
		DirectX::XMFLOAT3 fresnel_r0{0.01f, 0.01f, 0.01f};
		f32 roughness = 0.25f;
	};

	struct SFAPI Material {
		stl::string name;
		DirectX::XMFLOAT4 diffuse_albedo{1.f, 1.f, 1.f, 1.f};
		DirectX::XMFLOAT3 fresnel_r0{0.01f, 0.01f, 0.01f};
		f32 roughness = 0.25f;
        Buffer material_buffer{};
        s32 material_cb_index = -1;
	};
} // namespace Sapfire::d3d
