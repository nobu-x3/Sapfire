#pragma once

#include "core/core.h"
#include "core/uuid.h"

namespace Sapfire{

	constexpr size_t MAX_COMPONENTS = 128;
	using Signature = stl::bitset<MAX_COMPONENTS>;

	class SFAPI Entity {
	public:
		Entity() = default;
		Entity(UUID uuid, stl::generational_index index);
		Entity(stl::generational_index index);
		bool operator==(const Entity& other) const;
		inline stl::generational_index id() const { return m_Id; }
		inline void id(stl::generational_index id) { m_Id = id; }
		inline void uuid(UUID uuid) { m_UUID = uuid; }
		inline UUID uuid() const { return m_UUID; }

	private:
		UUID m_UUID;
		stl::generational_index m_Id;
	};

	constexpr u32 MAX_ENTITIES = 512;

	class SFAPI EntityRegistry {
	public:
		Entity create_entity();
		Entity create_entity(UUID id);
		void destroy_entity(Entity entity);
		void signature(Entity entity, Signature signature);
		bool is_valid(stl::generational_index index) const;
        bool is_valid(Entity entity) const;
		stl::vector<stl::generational_index> get_all_valid_entities() const;
		[[nodiscard]] Signature signature(Entity entity) const;
		[[nodiscard]] inline const stl::generational_vector<Entity>& entities() const { return m_Entities; }
		[[nodiscard]] stl::optional<Entity> entity(stl::generational_index valid_index);

	private:
		stl::vector<Signature> m_Signatures{};
		stl::generational_index_allocator m_Allocator{};
		stl::generational_vector<Entity> m_Entities{};
	};
} // namespace Sapfire
  //
namespace std {
	template <typename T>
	struct hash;

	template <>
	struct hash<Sapfire::Entity> {
		std::size_t operator()(const Sapfire::Entity& entity) const { return entity.uuid(); }
	};
} // namespace std
