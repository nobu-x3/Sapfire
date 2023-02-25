#pragma once

#include "Sapfire/scene/Scene.h"
#include "entt/entt.hpp"

namespace Sapfire
{
	class Entity
	{
	public:
		Entity(entt::entity id, Scene* scene);
		Entity(const Entity& other) = default;

		template <typename T, typename... Args>
		T& add_component(Args&&... args)
		{
			if (has_component<T>()) { ENGINE_ERROR("Entity {0} already has component", mEntityId); }
			return mScene->mRegistry.emplace<T>(mEntityId, std::forward<Args>(args)...);
		}

		template <typename T>
		T& get_component()
		{
			if (!has_component<T>()) { ENGINE_ERROR("Entity {0} does not have component!", mEntityId); }
			return mScene->mRegistry.get<T>(mEntityId);
		}

		template <typename T>
		T& remove_component()
		{
			if (!has_component<T>()) { ENGINE_ERROR("Entity {0} does not have component!", mEntityId); }
			return mScene->mRegistry.remove<T>(mEntityId);
		}
		
		template <typename T>
		bool has_component()
		{
			return mScene->mRegistry.any_of<T>(mEntityId);
		}

	private:
		entt::entity mEntityId;
		Scene* mScene; // 12 bytes so whatever
	};
}
