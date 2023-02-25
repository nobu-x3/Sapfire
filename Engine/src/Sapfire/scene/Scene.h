#pragma once

#include <entt/entt.hpp>

namespace Sapfire
{
	class Scene
	{
	public:
		Scene();
		~Scene();
		void on_update(float deltaTime);
		entt::entity create_entity();
		inline entt::registry& get_registry() { return mRegistry; }

	private:
		entt::registry mRegistry;
	};
}
