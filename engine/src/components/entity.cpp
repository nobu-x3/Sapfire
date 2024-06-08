#include "engpch.h"

#include "components/entity.h"

namespace Sapfire {

	Entity::Entity(UUID uuid, stl::generational_index id) : m_UUID(uuid), m_Id(id) {}

	Entity::Entity(stl::generational_index id) : m_Id(id), m_UUID() {}

	bool Entity::operator==(const Entity& other) const { return m_UUID == other.m_UUID; }

	Entity EntityRegistry::create_entity() {
		auto index = m_Allocator.allocate();
		Entity entity{index};
		m_Entities.set(index, entity);
		m_Signatures.push_back({});
		return entity;
	}

	Entity EntityRegistry::create_entity(UUID id) {
		auto index = m_Allocator.allocate();
		Entity entity{id, index};
		m_Entities.set(index, entity);
		m_Signatures.push_back({});
		return entity;
	}

	void EntityRegistry::destroy_entity(Entity entity) {
		m_Allocator.deallocate(entity.id());
		m_Entities.remove(entity.id());
	}

	stl::vector<stl::generational_index> EntityRegistry::get_all_valid_entities() const {
		return m_Entities.get_all_valid_indices(m_Allocator);
	}

	bool EntityRegistry::is_valid(stl::generational_index index) const {
		return index.index < m_Entities.size() && m_Allocator.is_alive(index);
	}

	bool EntityRegistry::is_valid(Entity entity) const {
		u32 index = 0;
		for (auto& entry : m_Entities) {
			if (!entry.has_value())
				continue;
			if (entry->value == entity) {
				return m_Allocator.is_alive({.index = index, .generation = entry->generation});
			}
			index++;
		}
		return false;
	}

	void EntityRegistry::signature(Entity entity, Signature signature) { m_Signatures[entity.id().index] = signature; }

	Signature EntityRegistry::signature(Entity entity) const { return m_Signatures[entity.id().index]; }

	stl::optional<Entity> EntityRegistry::entity(stl::generational_index valid_index) {
		auto* entity = m_Entities.get(valid_index);
		if (!entity)
			return {};
		return {*entity};
	}
} // namespace Sapfire
