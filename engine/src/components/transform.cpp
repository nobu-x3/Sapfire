#include "engpch.h"

#include <DirectXMath.h>
#include "components/transform.h"
#include "core/rtti.h"
#include "tools/profiling.h"

namespace Sapfire::components {

	ENGINE_COMPONENT_IMPL(Transform);

	Transform::Transform(const Transform& other) {
		m_Transform = other.m_Transform;
		m_Rotation = other.m_Rotation;
		m_Position = other.m_Position;
		m_Scale = other.m_Scale;
		m_Right = other.m_Right;
		m_Forward = other.m_Forward;
		m_Up = other.m_Up;
		m_EulerAngles = other.m_EulerAngles;
		m_RotationMatrix = other.m_RotationMatrix;
		m_ParentIndex = other.m_ParentIndex;
		m_Dirty = other.m_Dirty;
		register_rtti();
	}

	Transform::Transform(Transform&& other) noexcept {
		m_Transform = std::move(other.m_Transform);
		m_Rotation = std::move(other.m_Rotation);
		m_Position = std::move(other.m_Position);
		m_Scale = std::move(other.m_Scale);
		m_Right = std::move(other.m_Right);
		m_Forward = std::move(other.m_Forward);
		m_Up = std::move(other.m_Up);
		m_EulerAngles = std::move(other.m_EulerAngles);
		m_RotationMatrix = std::move(other.m_RotationMatrix);
		m_ParentIndex = std::move(other.m_ParentIndex);
		m_Dirty = std::move(other.m_Dirty);
		register_rtti();
	}

	Transform& Transform::operator=(const Transform& other) {
		m_Transform = other.m_Transform;
		m_Rotation = other.m_Rotation;
		m_Position = other.m_Position;
		m_Scale = other.m_Scale;
		m_Right = other.m_Right;
		m_Forward = other.m_Forward;
		m_Up = other.m_Up;
		m_EulerAngles = other.m_EulerAngles;
		m_RotationMatrix = other.m_RotationMatrix;
		m_ParentIndex = other.m_ParentIndex;
		m_Dirty = other.m_Dirty;
		register_rtti();
		return *this;
	}

	Transform& Transform::operator=(Transform&& other) noexcept {
		m_Transform = std::move(other.m_Transform);
		m_Rotation = std::move(other.m_Rotation);
		m_Position = std::move(other.m_Position);
		m_Scale = std::move(other.m_Scale);
		m_Right = std::move(other.m_Right);
		m_Forward = std::move(other.m_Forward);
		m_Up = std::move(other.m_Up);
		m_EulerAngles = std::move(other.m_EulerAngles);
		m_RotationMatrix = std::move(other.m_RotationMatrix);
		m_ParentIndex = std::move(other.m_ParentIndex);
		m_Dirty = std::move(other.m_Dirty);
		register_rtti();
		return *this;
	}

	void Transform::register_rtti() {
		BEGIN_RTTI()
		ADD_RTTI_FIELD(rtti ::rtti_type ::XMVECTOR, "Position", &m_Position, [this]() { m_Dirty = true; })
		ADD_RTTI_FIELD(rtti ::rtti_type ::XMVECTOR, "Scale", &m_Scale, [this]() { m_Dirty = true; })
		ADD_RTTI_FIELD(rtti ::rtti_type ::XMVECTOR, "Euler Rotation", &m_EulerAngles, [this]() { this->euler_rotation(m_EulerAngles); })
		END_RTTI()
	}

	void Transform::update(stl::vector<Transform>& transforms) {
		PROFILE_FUNCTION();
		/* auto scale = XMMatrixScalingFromVector(m_Scale); */
		/* m_RotationMatrix = XMMatrixRotationQuaternion(m_Rotation); */
		/* auto transl = XMMatrixTranslationFromVector(m_Position); */
		m_Transform = XMMatrixAffineTransformation(m_Scale, m_Position, m_Rotation, m_Position);
		/* m_Transform = scale * m_RotationMatrix * transl; */
		if (m_ParentIndex >= 0) {
			m_Transform = transforms[m_ParentIndex].m_Transform * m_Transform;
		}
	}

	Transform& Transform::position(XMVECTOR position) {
		PROFILE_FUNCTION();
		m_Position = position;
		m_Dirty = true;
		return *this;
	}

	Transform& Transform::rotation(XMVECTOR rotation) {
		PROFILE_FUNCTION();
		m_Rotation = rotation;
		m_RotationMatrix = XMMatrixRotationQuaternion(rotation);
		m_Forward = XMVector3Rotate(XMVECTOR{0, 0, 1, 0}, m_Rotation);
		m_Right = XMVector3Rotate(XMVECTOR{1, 0, 0, 0}, m_Rotation);
		m_Up = XMVector3Normalize(XMVector3Cross(m_Right, m_Forward));
		m_Dirty = true;
		return *this;
	}

	Transform& Transform::euler_rotation(XMVECTOR euler_rotation) {
		PROFILE_FUNCTION();
		m_EulerAngles = euler_rotation;
		m_Rotation = XMQuaternionRotationRollPitchYawFromVector(m_EulerAngles);
		m_RotationMatrix = XMMatrixRotationQuaternion(m_Rotation);
		m_Forward = XMVectorSet(0, 0, 1, 0);
		m_Forward = XMVector4Transform(m_Forward, m_RotationMatrix);
		m_Right = XMVectorSet(1, 0, 0, 0);
		m_Right = XMVector4Transform(m_Right, m_RotationMatrix);
		/* m_Forward = XMVector4Rotate(XMVectorSet(0,0,1,0), m_Rotation); */
		/* m_Right = XMVector3Rotate(XMVectorSet(1,0,0,0), m_Rotation); */
		m_Up = XMVector3Normalize(XMVector3Cross(m_Right, m_Forward));
		m_Dirty = true;
		return *this;
	}

	Transform& Transform::scale(XMVECTOR scale) {
		PROFILE_FUNCTION();
		m_Scale = scale;
		return *this;
	}

} // namespace Sapfire::components
