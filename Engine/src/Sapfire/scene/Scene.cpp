#include "engpch.h"
#include "Scene.h"
#include "Sapfire/scene/Components.h"
#include "entt/entt.hpp"
#include "Sapfire/renderer/Renderer.h"

namespace Sapfire
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::on_update(float deltaTime)
	{
		auto group = mRegistry.group<TransformComponent>(entt::get<MeshRendererComponent>);
		for(auto entity : group)
		{
			auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
			Renderer::submit_mesh(mesh.Mesh3D, transform.Transform);
		}
	}

	entt::entity Scene::create_entity()
	{
		return mRegistry.create();
	}
}
