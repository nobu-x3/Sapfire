#pragma once
#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Sapfire/renderer/Mesh.h"

namespace Sapfire
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent &) = default;

		TagComponent(const std::string &name) :
			Tag(name)
		{
		}

		operator std::string&() { return Tag; }
		operator const std::string&() { return Tag; }
	};

	struct TransformComponent
	{
		glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

	private:
		glm::vec3 EulerRotation = {0.0f, 0.0f, 0.0f};
		glm::quat Rotation{1.f, 0.f, 0.f, 0.f};

	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent &) = default;

		TransformComponent(const glm::vec3 &translation) :
			Translation(translation)
		{
		}

		glm::mat4 get_transform() const
		{
			return glm::translate(glm::mat4(1.f), -Translation) * glm::mat4_cast(Rotation) *
				glm::scale(glm::mat4(1.f), Scale);
		}

		glm::quat get_orientation() const
		{
			return {glm::vec3(-Rotation.x, -Rotation.y, Rotation.z)};
		}

		glm::vec3 get_euler_rotation() const { return EulerRotation; }

		void set_euler_rotation(const glm::vec3 &euler)
		{
			EulerRotation = euler;
			Rotation = glm::quat(EulerRotation);
		}

		glm::quat get_rotation() const { return Rotation; }

		void set_rotation(const glm::quat &quat)
		{
			Rotation = quat;
			EulerRotation = glm::eulerAngles(quat);
		}

		glm::vec3 get_forward_vector() const
		{
			return {glm::rotate(Rotation, {0.f, 0.f, -1.f})};
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
		SceneCamera Camera;
		bool IsActive = true;
		CameraComponent() = default;
		CameraComponent(const CameraComponent &cameraComponent) = default;

		CameraComponent(float degFov, float width, float height, float nearP, float farP)
		{
			Camera.set_perspective_projection_matrix(glm::radians(degFov), width, height, nearP, farP);
		}
	};

	struct ScriptComponent
	{
		ScriptableEntity *Instance = nullptr;

		ScriptableEntity *(*init_script)();
		void (*destroy_script)(ScriptComponent *);

		template <typename T>
		void bind()
		{
			init_script = []() { return static_cast<ScriptableEntity *>(new T()); };
			destroy_script = [](ScriptComponent *comp)
			{
				delete comp->Instance;
				comp->Instance = nullptr; // just to be sure
			};
		}
	};
}
