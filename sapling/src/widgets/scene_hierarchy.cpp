#include "Sapfire.h"

#include "components/name_component.h"
#include "components/transform.h"
#include "core/core.h"
#include "events/input_event.h"
#include "imgui.h"
#include "widgets/scene_hierarchy.h"

using namespace Sapfire;

namespace widgets {
	SSceneHierarchy::SSceneHierarchy(Sapfire::ECManager* ec_manager, EntitySelectedCallback callback) :
		m_ECManager(*ec_manager), m_ShowContextMenu(false), m_EntitySelectedCallback(callback) {}

	bool SSceneHierarchy::update(f32 delta_time) {
		if (ImGui::Begin("Scene Hierarchy")) {
			auto valid_indices = m_ECManager.get_all_valid_entities();
			ImGui::PushID("NO_PARENT");
			// This is a work around to impossibility of having a DnD target without an item
			if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
						IM_ASSERT(payload->DataSize == sizeof(Entity));
						const Entity entity = *(const Entity*)payload->Data;
						auto& transform_component = m_ECManager.engine_component<components::Transform>(entity);
						transform_component.parent(-1);
					}
					ImGui::EndDragDropTarget();
				}
				if (ImGui::IsItemHovered() && m_ShowContextMenu) {
					ImGui::OpenPopup("scene_hierarchy_context_menu_for_entity");
				}
				if (ImGui::BeginPopup("scene_hierarchy_context_menu_for_entity")) {
					if (ImGui::MenuItem("Create")) {
						auto entity = m_ECManager.create_entity();
						auto& transform_component = m_ECManager.engine_component<components::Transform>(entity);
						transform_component.parent(-1);
						m_ShowContextMenu = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				// Go through all valid entities and make tree nodes recursively
				for (u32 i = 0; i < valid_indices.size(); ++i) {
					stl::generational_index parent_index = valid_indices[i];
					const auto& maybe_parent_entity = m_ECManager.entity(parent_index);
					if (maybe_parent_entity.has_value()) {
						const auto& parent_entity = maybe_parent_entity.value();
						auto& transform_component = m_ECManager.engine_component<components::Transform>(parent_entity);
						// Tree roots will have transform parent index == -1
						if (transform_component.parent() == -1) {
							build_tree_for_entity(valid_indices, i);
						}
					}
				}
				ImGui::TreePop();
			}
			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_SelectEntity) {
				m_SelectedEntity = {};
				m_EntitySelectedCallback(m_SelectedEntity);
				m_SelectEntity = false;
			}
			ImGui::PopID();
		}
		ImGui::End();
		return true;
	}

	void SSceneHierarchy::build_tree_for_entity(const stl::vector<stl::generational_index>& indices, u32 valid_parent_index) {
		// Get parent entity
		stl::generational_index parent_index = indices[valid_parent_index];
		const auto& maybe_parent_entity = m_ECManager.entity(parent_index);
		if (!maybe_parent_entity.has_value())
			return;
		const auto& parent_entity = maybe_parent_entity.value();
		auto& name_component = m_ECManager.engine_component<Sapfire::components::NameComponent>(parent_entity);
		// Make a tree node for the parent entity
		auto flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		if (m_SelectedEntity.has_value() && m_SelectedEntity.value() == parent_entity) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (ImGui::TreeNodeEx(name_component.name().c_str(), flags)) {
			ImGui::PushID(name_component.name().c_str());
			// DnD stuff
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				ImGui::SetDragDropPayload("DND_ENTITY", &parent_entity, sizeof(Entity));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
					IM_ASSERT(payload->DataSize == sizeof(Entity));
					const Entity payload_entity = *(const Entity*)payload->Data;
					auto& transform_component = m_ECManager.engine_component<components::Transform>(payload_entity);
					transform_component.parent(parent_entity.id().index);
				}
				ImGui::EndDragDropTarget();
			}
			if (ImGui::IsItemHovered() && m_SelectEntity) {
				m_EntitySelectedCallback(parent_entity);
				m_SelectedEntity = parent_entity;
				m_SelectEntity = false;
			}
			draw_entity_context_menu(parent_entity);
			// Iterate over all entities again in case we reparented someone
			for (u32 next_valid_index = 0; next_valid_index < indices.size(); ++next_valid_index) {
				auto& next_index = indices[next_valid_index];
				auto maybe_next_entity = m_ECManager.entity(next_index);
				if (!maybe_next_entity.has_value())
					continue;
				const auto& next_entity = maybe_next_entity.value();
				auto& transform_component = m_ECManager.engine_component<components::Transform>(next_entity);
				// Only call recursively from this stack if that entity is a child of this entity
				if (transform_component.parent() == parent_index.index) {
					build_tree_for_entity(indices, next_valid_index);
				}
			}
			ImGui::PopID();
			ImGui::TreePop();
		}
	}

	void SSceneHierarchy::draw_entity_context_menu(const Sapfire::Entity& parent_entity) {
		// Right clicking will open a "context menu" as a popup
		if (ImGui::IsItemHovered() && m_ShowContextMenu) {
			ImGui::OpenPopup("scene_hierarchy_context_menu_for_entity");
		}
		if (ImGui::BeginPopup("scene_hierarchy_context_menu_for_entity")) {
			if (ImGui::MenuItem("Create")) {
				auto entity = m_ECManager.create_entity();
				auto& transform_component = m_ECManager.engine_component<components::Transform>(entity);
				transform_component.parent(parent_entity.id().index);
				m_ShowContextMenu = false;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Remove")) {
				// destroy_entity() takes care of children
				m_ECManager.destroy_entity(parent_entity);
				m_ShowContextMenu = false;
				if (m_SelectedEntity.has_value() && m_SelectedEntity.value() == parent_entity) {
					m_SelectedEntity = {};
					m_EntitySelectedCallback(m_SelectedEntity);
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void SSceneHierarchy::on_mouse_button_event(Sapfire::MouseButtonEvent& event) {
		if (!event.is_down()) {
			switch (event.button()) {
			case Sapfire::MouseButton::RMB:
				m_ShowContextMenu = true;
				break;
			default:
				break;
			}
		}
		// This will be called after internal ImGui selects and such.
		// @NOTE: Probably why it's sometimes laggy and button clicks don't do anything.
		if (event.is_down()) {
			m_ShowContextMenu = false;
			switch (event.button()) {
			case Sapfire::MouseButton::LMB:
				m_SelectEntity = true;
				break;
			default:
				break;
			}
		}
	}
} // namespace widgets
