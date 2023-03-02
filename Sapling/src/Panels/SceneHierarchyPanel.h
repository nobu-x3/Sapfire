#pragma once

#include "Sapfire/core/Core.h"
#include "Sapfire/scene/Entity.h"

namespace Sapfire
{
	class Scene;
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);
		SceneHierarchyPanel() = default;
		void set_context(const Ref<Scene>& scene);
		void OnImguiRender();
	private:
		void draw_entity_tree(Entity entity);
		void draw_components(Entity entity);
	private:
		Ref<Scene> mContext;
		Entity mSelectionIndex;
	};
}
