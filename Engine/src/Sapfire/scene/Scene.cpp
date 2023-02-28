#include "engpch.h"
#include "Scene.h"
#include "Sapfire/scene/Components.h"
#include "entt/entt.hpp"
#include "Sapfire/renderer/Renderer.h"
#include "Sapfire/scene/Entity.h"
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
	Scene::Scene()
	{
		BufferLayout matrixUniBufLayout = {{"view", ShaderDataType::Mat4}, {"proj", ShaderDataType::Mat4}};
		mUniformBuffer = UniformBuffer::create(0, matrixUniBufLayout);
	}

	Scene::~Scene()
	{
	}

	void Scene::on_update(float deltaTime)
	{
		// update scripts
		{
			mRegistry.view<ScriptComponent>().each([=](auto entity, auto& scriptComp)
			{
				// TODO: move this to on_scene_play() or smth
				if(!scriptComp.Instance)
				{
					scriptComp.Instance = scriptComp.init_script();
					scriptComp.Instance->mEntity = Entity {entity, this};
					scriptComp.Instance->on_create();
				}
				scriptComp.Instance->on_update(deltaTime);
			});
		}
		CameraComponent *mainCamera = nullptr;
		TransformComponent *cameraTransform = nullptr;
		{
			auto group = mRegistry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group)
			{
				auto [camera, transform] = group.get<CameraComponent, TransformComponent>(entity);
				if (camera.IsActive)
				{
					mainCamera = &camera;
					cameraTransform = &transform;
				}
			}
		}
		if (mainCamera)
		{
			mainCamera->Camera.set_viewport_size(mViewportWidth, mViewportHeight);
			Renderer::begin_scene(mainCamera->Camera, cameraTransform->get_transform(), mUniformBuffer);
			{
				auto group = mRegistry.group<TransformComponent>(entt::get<MeshRendererComponent>);
				for (auto entity : group)
				{
					auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
					Renderer::submit_mesh(mesh.Mesh3D, transform.get_transform());
				}
			}
			Renderer::end_scene();
		}
	}

	void Scene::set_viewport_size(uint32_t width, uint32_t height)
	{
		mViewportWidth = width;
		mViewportHeight = height;
	}

	Entity Scene::create_entity()
	{
		Entity entity = {mRegistry.create(), this};
		entity.add_component<TransformComponent>();
		return entity;
	}
}
