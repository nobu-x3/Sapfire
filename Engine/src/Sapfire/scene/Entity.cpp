#include "engpch.h"
#include "Entity.h"

namespace Sapfire
{
	Entity::Entity(entt::entity id, Scene* scene) : mEntityId(id), mScene(scene)
	{
	}
}
