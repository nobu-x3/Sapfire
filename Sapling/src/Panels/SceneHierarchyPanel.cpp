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
		mContext->mRegistry.each([&](auto entityId)
		{
			Entity entity = {entityId, mContext.get()};
			draw_entity_tree(entity);
		});
		ImGui::End();
	}

	void SceneHierarchyPanel::draw_entity_tree(Entity entity)
	{
			auto& tag = entity.get_component<TagComponent>().Tag;
		ImGuiTreeNodeFlags flags = ((mSelectionIndex == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, tag.c_str());
		if(ImGui::IsItemClicked())
		{
			mSelectionIndex = entity;
		}
		if(opened)
		{
			ImGui::TreePop();
		}
	}
}
