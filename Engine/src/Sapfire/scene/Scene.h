#pragma once

#include <entt/entt.hpp>
#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class UniformBuffer;

	class Scene
	{
		friend class Entity;
		friend class SceneHierarchyPanel;

	public:
		Scene();
		~Scene();
		void on_update(float deltaTime);
		void set_viewport_size(uint32_t width, uint32_t height);
		Entity create_entity(const std::string& name = "");

	private:
		entt::registry mRegistry;
		Ref<UniformBuffer> mUniformBuffer;
		uint32_t mViewportWidth;
		uint32_t mViewportHeight;
	};
}
