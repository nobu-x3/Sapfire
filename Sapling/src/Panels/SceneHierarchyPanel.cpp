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
		if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) // deselection
			mSelectionIndex = {};
		ImGui::End();
		ImGui::Begin("Inspector");
		if (mSelectionIndex)
		{
			draw_components(mSelectionIndex);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::draw_entity_tree(Entity entity)
	{
		auto &tag = entity.get_component<TagComponent>().Tag;
		ImGuiTreeNodeFlags flags = ((mSelectionIndex == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64_t>(static_cast<uint32_t>(entity))),
		                                flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			mSelectionIndex = entity;
		}
		if (opened)
		{
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::draw_components(Entity entity)
	{
		if (entity.has_component<TagComponent>())
		{
			if(ImGui::CollapsingHeader("Tag"))
			{
				auto &tag = entity.get_component<TagComponent>().Tag;
				char buffer[256] = {};
				strcpy_s(buffer, sizeof(buffer), tag.c_str());
				if (ImGui::InputText("", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}
			}
		}
		if (entity.has_component<TransformComponent>())
		{
			if(ImGui::CollapsingHeader("Transform"))
			{
				auto &transform = entity.get_component<TransformComponent>();
				ImGui::DragFloat3("Position",
								  glm::value_ptr(transform.Translation), 0.1f);
				ImGui::DragFloat3("Scale", value_ptr(transform.Scale), 0.1f);
				auto rotation = transform.get_euler_rotation();
				if(ImGui::DragFloat3("Rotation", value_ptr(rotation), 0.1f))
				{
					transform.set_euler_rotation(rotation);
				}
			}
		}
	}
}
