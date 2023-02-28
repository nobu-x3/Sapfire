#include "engpch.h"
#include "Entity.h"
#include "Sapfire/scene/Components.h"
namespace Sapfire
{
	Entity::Entity(entt::entity id, Scene *scene) :
		mEntityId(id), mScene(scene)
	{
	}

	TransformComponent &Entity::transform()
	{
		return get_component<TransformComponent>();
	}
}
