#include "engpch.h"

#include "components/ec_manager.h"
#include "components/name_component.h"
#include "components/transform.h"
#include "core/memory.h"

namespace Sapfire {

	ECManager::ECManager() :
		m_ComponentRegistry(stl::make_unique<components::ComponentRegistry>(mem::ENUM::Engine_Components)),
		m_EntityRegistry(stl::make_unique<EntityRegistry>(mem::ENUM::Engine_Components)) {}

	Entity ECManager::create_entity() {
		auto entity = m_EntityRegistry->create_entity();
		m_ComponentRegistry->add_engine_component<components::NameComponent>(entity, {});
		m_ComponentRegistry->add_engine_component<components::Transform>(entity, {});
		return entity;
	}

	void ECManager::destroy_entity(Entity entity) {
		m_EntityRegistry->destroy_entity(entity);
		auto indices = m_EntityRegistry->entities();
		for (auto& index : indices) {
			if (!index.has_value())
				continue;
			auto entity_to_change = index->value;
			auto& transform = m_ComponentRegistry->get_engine_component<components::Transform>(entity_to_change);
			if (transform.parent() == entity_to_change.id().index) {
				destroy_entity(entity_to_change);
			}
		}
		m_ComponentRegistry->entity_destroyed(entity);
	}
} // namespace Sapfire
