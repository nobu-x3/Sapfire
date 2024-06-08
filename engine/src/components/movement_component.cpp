#include "engpch.h"

#include "components/movement_component.h"
#include "core/rtti.h"

namespace Sapfire::components {
	ENGINE_COMPONENT_IMPL(MovementComponent);

	MovementComponent::MovementComponent() { register_rtti(); }

	MovementComponent::MovementComponent(const MovementComponent& other) {
		m_Velocity = other.m_Velocity;
		m_Acceleration = other.m_Acceleration;
		register_rtti();
	}

	MovementComponent::MovementComponent(MovementComponent&& other) noexcept {
		m_Velocity = std::move(other.m_Velocity);
		m_Acceleration = std::move(other.m_Acceleration);
		register_rtti();
	}

	MovementComponent& MovementComponent::operator=(const MovementComponent& other) {
		m_Velocity = other.m_Velocity;
		m_Acceleration = other.m_Acceleration;
		register_rtti();
		return *this;
	}

	MovementComponent& MovementComponent::operator=(MovementComponent&& other) noexcept {
		m_Velocity = std::move(other.m_Velocity);
		m_Acceleration = std::move(other.m_Acceleration);
		register_rtti();
		return *this;
	}

	void MovementComponent::register_rtti() {
		BEGIN_RTTI();
		ADD_RTTI_FIELD(rtti::rtti_type::XMVECTOR, "Acceleration", &m_Acceleration, nullptr);
		ADD_RTTI_FIELD(rtti::rtti_type::XMVECTOR, "Velocity", &m_Velocity, nullptr);
		END_RTTI();
	}

} // namespace Sapfire
