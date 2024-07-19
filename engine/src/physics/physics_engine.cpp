#include "engpch.h"
#include <DirectXMath.h>
#include "components/ec_manager.h"
#include "components/movement_component.h"
#include "components/transform.h"
#include "physics/physics_engine.h"

namespace Sapfire::physics {

	using namespace DirectX;

	PhysicsEngine::PhysicsEngine(ECManager* ec_manager) : m_ECManager(*ec_manager) {}

	void PhysicsEngine::simulate(f32 delta_time) {
		auto& entries = m_ECManager.entities();
		u32 index = -1;
		for (auto& entry : entries) {
			index++;
			if (!entry.has_value())
				continue;
			const stl::generational_index gen_index = {.index = index, .generation = entry->generation};
			if (!m_ECManager.is_valid(gen_index))
				continue;
			auto entity = m_ECManager.entity(gen_index);
			if (!m_ECManager.has_engine_component<components::MovementComponent>(entity.value()))
				continue;
			auto& transform = m_ECManager.engine_component<components::Transform>(entity.value());
			auto& movement = m_ECManager.engine_component<components::MovementComponent>(entity.value());
			const f32 damping = 1.0f;
			auto velocity = movement.velocity();
			velocity += movement.acceleration() * delta_time;
			velocity *= damping;
			movement.velocity(velocity);
			auto position = transform.position();
			position += velocity * delta_time;
			transform.position(position);
		}
	}
} // namespace Sapfire::physics
