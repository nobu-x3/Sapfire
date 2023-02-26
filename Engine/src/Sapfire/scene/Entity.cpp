#include "engpch.h"
#include "Entity.h"

#include "Components.h"
#include "glm/gtx/matrix_transform_2d.hpp"

namespace Sapfire
{
	Entity::Entity(entt::entity id, Scene* scene) : mEntityId(id), mScene(scene)
	{
	}

	void Entity::set_position(const glm::vec3& pos) const
	{
		auto& transform = get_component<TransformComponent>().Transform;
		transform = glm::translate(glm::mat4(1.f), -pos);
	}

	void Entity::set_rotation(const glm::quat& quat) const
	{
		auto& transform = get_component<TransformComponent>().Transform;
		transform *= glm::mat4_cast(quat);
	}

	void Entity::set_scale(const glm::vec3& scale) const
	{
		auto& transform = get_component<TransformComponent>().Transform;
		transform *= glm::scale(glm::mat4(1.f), scale);
	}

	void Entity::translate(const glm::vec3& offset) const
	{
		auto& transform = get_component<TransformComponent>().Transform;
		transform = glm::translate(transform, offset);
	}
}
