#include "engpch.h"
#include "Scene.h"
#include "entt/entt.hpp"

namespace Sapfire
{
	Scene::Scene()
	{
		struct TransformComponent
		{
			glm::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const TransformComponent&) = default;
			TransformComponent(const glm::mat4& transform) : Transform(transform) {}
		};
		entt::entity entity = mRegistry.create();
		mRegistry.emplace<TransformComponent>(entity, glm::mat4(1.f));
	}

	Scene::~Scene()
	{
	}
}