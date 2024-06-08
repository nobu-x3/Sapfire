#pragma once

#include <DirectXMath.h>
#include "components/component.h"

namespace Sapfire::components {
	class MovementComponent {
		RTTI;
		ENGINE_COMPONENT(MovementComponent)
	public:
		MovementComponent();
		MovementComponent(const MovementComponent&);
		MovementComponent(MovementComponent&&) noexcept;
		MovementComponent& operator=(const MovementComponent&);
		MovementComponent& operator=(MovementComponent&&) noexcept;

		inline DirectX::XMVECTOR acceleration() const { return m_Acceleration; }
		inline void acceleration(DirectX::XMVECTOR acceleration) { m_Acceleration = acceleration; }
		inline DirectX::XMVECTOR velocity() const { return m_Velocity; }
		inline void velocity(DirectX::XMVECTOR velocity) { m_Velocity = velocity; }

    private:
        void register_rtti();

	private:
		DirectX::XMVECTOR m_Acceleration{0.0f, 0.0f, 0.0f, 0.0f};
		DirectX::XMVECTOR m_Velocity{0.0f, 0.0f, 0.0f, 0.0f};
	};
} // namespace Sapfire
