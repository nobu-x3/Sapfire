#include "engpch.h"

#include <DirectXMath.h>
#include "render/camera.h"

namespace Sapfire {

	using namespace DirectX;

	Camera::Camera(f32 fov, f32 aspect, f32 near_plane, f32 far_plane) {
		projection = DirectX::XMMatrixPerspectiveFovLH(fov * DirectX::XM_PI, aspect, near_plane, far_plane);
	}

	DirectX::XMMATRIX Camera::view() const {
		PROFILE_FUNCTION();
		auto translation_rotation =
			XMMatrixIdentity() * transform.rotation_matrix() * DirectX::XMMatrixTranslationFromVector(transform.position());
		auto determinant = DirectX::XMMatrixDeterminant(translation_rotation);
		return DirectX::XMMatrixInverse(&determinant, translation_rotation);
	}

	void Camera::update(f32 delta_time) {
		PROFILE_FUNCTION();
		if (input.mouse_state.RMB) {
			const XMFLOAT4 mouse_delta{-input.mouse_delta_y, -input.mouse_delta_x, 0.f, 0.f};
			auto euler = transform.euler_rotation();
			euler += XMLoadFloat4(&mouse_delta);
			transform.euler_rotation(euler);
		}
		auto velocity = movement_component.velocity();
		velocity = input.input_axis.x * transform.forward() + input.input_axis.y * transform.right() + input.input_axis.z * transform.up();
		movement_component.velocity(velocity);
		// movement_component.velocity *= delta_time;
		auto position = transform.position();
		position += velocity * delta_time;
		transform.position(position);
	}
} // namespace Sapfire
