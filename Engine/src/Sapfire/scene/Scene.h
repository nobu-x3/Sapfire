#pragma once

#include <entt/entt.hpp>

#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class UniformBuffer;

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
		Ref<UniformBuffer> mUniformBuffer;
	};
}
