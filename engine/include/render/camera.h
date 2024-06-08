#pragma once
#include <DirectXMath.h>
#include "components/movement_component.h"
#include "core/input.h"
#include "components/transform.h"

namespace Sapfire {
	struct SFAPI Camera {
		Camera() = default;
		Camera(f32 fov, f32 aspect, f32 near_plane, f32 far_plane);
		void update(f32);
		components::Transform transform{};
		DirectX::XMMATRIX projection;
		DirectX::XMMATRIX view() const;
        components::MovementComponent movement_component;
		input::InputComponent input;
		f32 fov;
		f32 aspect;
		f32 near_plane;
		f32 far_plane;
	};
} // namespace Sapfire
