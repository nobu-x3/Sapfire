#pragma once
#include "glm/glm.hpp"
#include "Sapfire/renderer/Camera.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/Shader.h"

namespace Sapfire
{
	struct TransformComponent
	{
		glm::mat4 Transform{1.f};

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(const glm::mat4& transform) : Transform(transform)
		{
		}
	};

	struct MeshRendererComponent
	{
		Mesh Mesh3D;

		MeshRendererComponent() = default;

		MeshRendererComponent(const Mesh& mesh) : Mesh3D(mesh)
		{
		}

		MeshRendererComponent(const std::string& meshPath, const Ref<Shader>& shader) : Mesh3D(meshPath, shader)
		{
		}
	};

	struct CameraComponent
	{
		Sapfire::Camera Camera;
		bool IsActive = true;
		CameraComponent() = default;

		CameraComponent(const Sapfire::Camera& camera) : Camera(camera)
		{
		}

		CameraComponent(const glm::mat4& projection) : Camera(projection)
		{
		}

		CameraComponent(float degFov, float width, float height, float nearP, float farP) : Camera(
			degFov, width, height, nearP, farP)
		{
		}
	};
}
