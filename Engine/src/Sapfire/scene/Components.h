#pragma once
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/rotate_normalized_axis.hpp"
#include "Sapfire/renderer/Camera.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/Shader.h"

namespace Sapfire
{
	struct TransformComponent
	{
		glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		TransformComponent() = default;
		TransformComponent(const TransformComponent &) = default;

		TransformComponent(const glm::vec3 &translation) :
			Translation(translation)
		{
		}

		glm::mat4 get_transform() const
		{
			return glm::translate(glm::mat4(1.f), -Translation) * glm::mat4_cast(glm::quat(glm::radians(Rotation))) *
				glm::scale(glm::mat4(1.f), Scale);
		}

		glm::quat get_orientation() const
		{
			return {glm::vec3(-Rotation.x, -Rotation.y, Rotation.z)};
		}

		glm::vec3 get_forward_vector() const
		{
			return {glm::rotate(get_orientation(), {1.f, 0.f, 0.f})};
		}
	};

	struct MeshRendererComponent
	{
		Mesh Mesh3D;

		MeshRendererComponent() = default;

		MeshRendererComponent(const Mesh &mesh) :
			Mesh3D(mesh)
		{
		}

		MeshRendererComponent(const std::string &meshPath, const Ref<Shader> &shader) :
			Mesh3D(meshPath, shader)
		{
		}
	};

	struct CameraComponent
	{
		Sapfire::Camera Camera;
		bool IsActive = true;
		CameraComponent() = default;

		CameraComponent(const Sapfire::Camera &camera) :
			Camera(camera)
		{
		}

		CameraComponent(const glm::mat4 &projection) :
			Camera(projection)
		{
		}

		CameraComponent(float degFov, float width, float height, float nearP, float farP) :
			Camera(
				degFov, width, height, nearP, farP)
		{
		}
	};
}
