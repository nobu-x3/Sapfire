#pragma once

#include <entt/entt.hpp>

namespace Sapfire
{
	class Scene
	{
		friend class Entity;

	public:
		Scene();
		~Scene();
		void on_update(float deltaTime);
		Entity create_entity();

	private:
		entt::registry mRegistry;
	};
}
