#include "engpch.h"

#include <DirectXMath.h>
#include "render/camera.h"

namespace Sapfire {

	using namespace DirectX;

	Camera::Camera(f32 fov, f32 aspect, f32 near_plane, f32 far_plane) :
		fov(fov), aspect(aspect), near_plane(near_plane), far_plane(far_plane) {
		projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, near_plane, far_plane);
	}

	DirectX::XMMATRIX Camera::view() const {
		PROFILE_FUNCTION();
		/* XMFLOAT4X4 view{}; */
        /* XMFLOAT3 mRight {}; */
        /* XMFLOAT3 mUp {}; */
        /* XMFLOAT3 mLook{}; */
		/* XMVECTOR R = transform.right(); */
		/* XMVECTOR U = transform.up(); */
		/* XMVECTOR L = transform.euler_rotation(); */
		/* XMVECTOR P = transform.position(); */
		/* // Keep camera's axes orthogonal to each other and of unit length. */
		/* L = XMVector3Normalize(L); */
		/* U = XMVector3Normalize(XMVector3Cross(L, R)); */
		/* // U, L already ortho-normal, so no need to normalize cross product. */
		/* R = XMVector3Cross(U, L); */
		/* // Fill in the view matrix entries. */
		/* float x = -XMVectorGetX(XMVector3Dot(P, R)); */
		/* float y = -XMVectorGetX(XMVector3Dot(P, U)); */
		/* float z = -XMVectorGetX(XMVector3Dot(P, L)); */
		/* XMStoreFloat3(&mRight, R); */
		/* XMStoreFloat3(&mUp, U); */
		/* XMStoreFloat3(&mLook, L); */
		/* view(0, 0) = mRight.x; */
		/* view(1, 0) = mRight.y; */
		/* view(2, 0) = mRight.z; */
		/* view(3, 0) = x; */
		/* view(0, 1) = mUp.x; */
		/* view(1, 1) = mUp.y; */
		/* view(2, 1) = mUp.z; */
		/* view(3, 1) = y; */
		/* view(0, 2) = mLook.x; */
		/* view(1, 2) = mLook.y; */
		/* view(2, 2) = mLook.z; */
		/* view(3, 2) = z; */
		/* view(0, 3) = 0.0f; */
		/* view(1, 3) = 0.0f; */
		/* view(2, 3) = 0.0f; */
		/* view(3, 3) = 1.0f; */
        /* return XMLoadFloat4x4(&view); */
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
		stl::vector<components::Transform> t{};
		transform.update(t);
	}
} // namespace Sapfire
