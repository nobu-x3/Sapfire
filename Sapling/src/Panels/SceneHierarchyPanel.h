#pragma once

#include "Sapfire/core/Core.h"

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
		Ref<Scene> mContext;
	};
}