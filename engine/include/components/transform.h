#pragma once
#include <DirectXMath.h>
#include "components/component.h"
#include "core/rtti.h"

namespace Sapfire::components {
	using namespace DirectX;

	class SFAPI Transform {
		RTTI;
		ENGINE_COMPONENT(Transform);

	public:
		inline Transform() { register_rtti(); }
		inline Transform(s32 parent_index) : m_ParentIndex(parent_index) { register_rtti(); }
		void register_rtti();
		Transform(const Transform&);
		Transform(Transform&&) noexcept;
		Transform& operator=(const Transform&);
		Transform& operator=(Transform&&) noexcept;
		void update(stl::vector<Transform>& transforms);
		inline XMMATRIX transform() const { return m_Transform; }
		inline XMVECTOR position() const { return m_Position; }
		inline XMVECTOR rotation() const { return m_Rotation; }
		inline XMVECTOR euler_rotation() const { return m_EulerAngles; }
		inline XMVECTOR scale() const { return m_Scale; }
		inline XMVECTOR forward() const { return m_Forward; }
		inline XMVECTOR up() const { return m_Up; }
		inline XMVECTOR right() const { return m_Right; }
		inline XMMATRIX rotation_matrix() const { return m_RotationMatrix; }
		inline s32 parent() const { return m_ParentIndex; }
		inline void parent(s32 parent_index) {
			m_ParentIndex = parent_index;
			m_Dirty = true;
		}
		Transform& position(XMVECTOR position);
		Transform& rotation(XMVECTOR rotation);
		Transform& euler_rotation(XMVECTOR euler_rotation);
		Transform& scale(XMVECTOR scale);

	private:
		XMMATRIX m_Transform{XMMatrixIdentity()};
		XMVECTOR m_Rotation{XMQuaternionIdentity()};
		XMVECTOR m_Position{0.0f, 0.0f, 0.0f, 0.0f};
		XMVECTOR m_Scale{1.f, 1.f, 1.f, 1.f};
		XMVECTOR m_Right{1.0f, 0.0f, 0.0f};
		XMVECTOR m_Forward{0.0f, 0.0f, 1.0f};
		XMVECTOR m_Up{0.0f, 1.0f, 0.0f};
		XMVECTOR m_EulerAngles{0.0f, 0.0f, 0.0f};
		XMMATRIX m_RotationMatrix{XMMatrixIdentity()};
		s32 m_ParentIndex{-1};
		bool m_Dirty{true};
	};
} // namespace Sapfire::components
