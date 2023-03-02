#include "SceneHierarchyPanel.h"
#include "Engine.h"
#include "imgui.h"

namespace Sapfire
{

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene)
	{
		set_context(scene);
	}

	void SceneHierarchyPanel::set_context(const Ref<Scene> &scene)
	{
		mContext = scene;
	}

	void SceneHierarchyPanel::OnImguiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		mContext->mRegistry.each([&](auto entity)
		{
			auto& tag = mContext->mRegistry.get<TagComponent>(entity);
			ImGui::Text("%s", tag.Tag.c_str());
		});
		ImGui::End();
	}
}
