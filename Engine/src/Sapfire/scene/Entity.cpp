#include "engpch.h"
#include "Entity.h"

#include "Components.h"
#include "glm/gtx/matrix_transform_2d.hpp"

namespace Sapfire
{
	Entity::Entity(entt::entity id, Scene* scene) : mEntityId(id), mScene(scene)
	{
	}
}
