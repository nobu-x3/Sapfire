#pragma once
#include "Sapfire/scene/Entity.h"

namespace Sapfire
{
	class ScriptableEntity
	{
		friend class Scene;

	public:
		virtual ~ScriptableEntity() = default;

		template <typename T>
		T &get_component() // betting on inlining
		{
			return mEntity.get_component<T>();
		}

	protected:
		virtual void on_create()
		{
		}

		virtual void on_destroy()
		{
		}

		virtual void on_update(float deltaTime)
		{
		}

		[[nodiscard]] const Sapfire::Entity &get_entity() const { return mEntity; }

	private:
		Sapfire::Entity mEntity;
	};
}
