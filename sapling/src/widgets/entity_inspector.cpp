#include "Sapfire.h"

#include "components/component.h"
#include "components/ec_manager.h"
#include "components/movement_component.h"
#include "components/name_component.h"
#include "components/render_component.h"
#include "core/core.h"
#include "imgui.h"
#include "rtti_drawer.h"
#include "widgets/entity_inspector.h"
#include "widgets/scene_view.h"
#include "widgets/tiny_widgets/buttons.h"

namespace widgets {

	using namespace Sapfire;
	SEntityInspector::SEntityInspector(Sapfire::ECManager* ec_manager, AddRenderComponentCallback callback) :
		m_ECManager(*ec_manager), m_AddRenderComponentCallback(callback) {}

	template <typename T>
	void draw_optional_engine_component(ECManager& ec_manager, stl::optional<Entity> entity) {
		if (!entity.has_value() || !ec_manager.is_valid(entity.value()))
			return;
		if (!ec_manager.has_engine_component<T>(entity.value()))
			return;
		auto& component = ec_manager.engine_component<T>(entity.value());
		ImGui::Text("%s", component.to_string().c_str());
		ImGui::SameLine();
		if (ImGui::Button("x")) {
			ec_manager.remove_engine_component<T>(entity.value());
		}
		draw_rtti<T>(&component);
	}

	bool SEntityInspector::update(f32 delta_time) {
		if (ImGui::Begin("Entity Inspector")) {
			if (m_SelectedEntity.has_value() && m_ECManager.is_valid(m_SelectedEntity.value())) {
				auto& name_component = m_ECManager.engine_component<components::NameComponent>(m_SelectedEntity.value());
				ImGui::Text("%s", name_component.to_string().c_str());
				draw_rtti<components::NameComponent>(&name_component);
				auto& transform_component = m_ECManager.engine_component<components::Transform>(m_SelectedEntity.value());
				ImGui::Text("%s", transform_component.to_string().c_str());
				draw_rtti<components::Transform>(&transform_component);
				draw_optional_engine_component<components::MovementComponent>(m_ECManager, m_SelectedEntity);
				draw_optional_engine_component<components::RenderComponent>(m_ECManager, m_SelectedEntity);
				auto entities_components = m_ECManager.components(m_SelectedEntity.value());
				for (auto& component : entities_components) {
					const char* component_name = component->to_string().c_str();
					ImGui::PushID(component_name);
					ImGui::Text("%s", component_name);
					ImGui::SameLine();
					if (ImGui::Button("x")) {
						m_ECManager.remove_component(m_SelectedEntity.value(), component);
					}
					draw_rtti(component);
					ImGui::PopID();
				}
				if (centered_button("+", {ImGui::GetContentRegionAvail().x / 4.f, 25})) {
					m_ShowAddComponentContextMenu = true;
				}
				if (m_ShowAddComponentContextMenu) {
					ImGui::OpenPopup("scene_hierarchy_context_menu_for_entity");
					if (ImGui::BeginPopup("scene_hierarchy_context_menu_for_entity")) {
						if (ImGui::Button(components::NameComponent::to_string().c_str())) {
							m_ECManager.add_engine_component<components::NameComponent>(m_SelectedEntity.value());
							m_ShowAddComponentContextMenu = false;
						}
						if (ImGui::Button(components::Transform::to_string().c_str())) {
							m_ECManager.add_engine_component<components::Transform>(m_SelectedEntity.value());
							m_ShowAddComponentContextMenu = false;
						}
						if (ImGui::Button(components::MovementComponent::to_string().c_str())) {
							m_ECManager.add_engine_component<components::MovementComponent>(m_SelectedEntity.value());
							m_ShowAddComponentContextMenu = false;
						}
						if (ImGui::Button(components::RenderComponent::to_string().c_str())) {
							m_AddRenderComponentCallback(m_SelectedEntity.value(),
														 {
															 .mesh_path = "assets/models/cube.obj",
															 .texture_path = "assets/textures/ceramics.jpg",
															 .material_path = "assets/materials/default.mat",
														 });
							m_ShowAddComponentContextMenu = false;
						}
						for (const auto& [key, component_list] : components::ComponentRegistry::s_CustomComponentLists) {
							components::ComponentType type = components::ComponentRegistry::s_ComponentTypes[key];
							stl::string component_list_name = component_list->to_string();
							if (ImGui::Button(component_list_name.c_str())) {
								m_ECManager.add_component(m_SelectedEntity.value(), type);
								m_ShowAddComponentContextMenu = false;
							}
						}
						ImGui::EndPopup();
						if (ImGui::IsAnyMouseDown() && !ImGui::IsAnyItemHovered() && m_ShowAddComponentContextMenu) {
							m_ShowAddComponentContextMenu = false;
						}
					}
				}
			}
		}
		ImGui::End();
		return true;
	}

	void SEntityInspector::select_entity(Sapfire::stl::optional<Sapfire::Entity> maybe_entity) { m_SelectedEntity = maybe_entity; }

} // namespace widgets
