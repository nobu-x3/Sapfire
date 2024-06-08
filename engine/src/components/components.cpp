#include "engpch.h"

#include "components/component.h"

namespace Sapfire::components {
	ComponentRegistry::ComponentRegistry() :
		m_NextComponentTypeNumber(s_NextComponentTypeNumber), m_ComponentTypes(s_ComponentTypes),
		m_EngineComponentLists(s_EngineComponentLists), m_ComponentTypeNameMap(s_ComponentTypeNameMap),
		m_CustomComponentLists(s_CustomComponentLists) {}

	CustomComponentList::CustomComponentList(const stl::shared_ptr<IComponent>& def_comp) : default_component(def_comp) {}

	stl::string CustomComponentList::to_string() { return default_component->to_string(); }

	void CustomComponentList::insert(Entity entity, stl::shared_ptr<IComponent> component) {
		if (m_EntityToIndexMap.size() > 0 && m_EntityToIndexMap.contains(entity)) {
			remove(entity);
		}
		m_EntityToIndexMap[entity] = m_Components.size();
		m_IndexToEntityMap[m_Components.size()] = entity;
		m_Components.push_back(component);
	}

	void CustomComponentList::remove(Entity entity) {
		if (m_Components.size() <= 0)
			return;
		// swap element at end to deleted element's place to maintain density
		auto removed_entity_index = m_EntityToIndexMap[entity];
		auto index_of_last_entity = m_Components.size() - 1;
		Entity last_entity = m_IndexToEntityMap[index_of_last_entity];
		std::swap(m_Components[removed_entity_index], m_Components[m_Components.size() - 1]);
		// update maps
		m_EntityToIndexMap[last_entity] = removed_entity_index;
		m_IndexToEntityMap[removed_entity_index] = last_entity;
		m_EntityToIndexMap.erase(entity);
		m_IndexToEntityMap.erase(index_of_last_entity);
		m_Components.pop_back();
	}

	stl::shared_ptr<IComponent> CustomComponentList::get(Entity entity) { return m_Components[m_EntityToIndexMap[entity]]; }

	void CustomComponentList::entity_destroyed(Entity entity) {
		if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()) {
			remove(entity);
		}
	}

	void ComponentRegistry::add_component(Entity entity, stl::shared_ptr<IComponent> component) {
		const char* type_name = m_ComponentTypeNameMap[component->component_type()];
		stl::shared_ptr<CustomComponentList>& component_list = m_CustomComponentLists[type_name];
		if (!component_list)
			component_list = stl::make_shared<CustomComponentList>(mem::ENUM::Game_Components);
		component_list->insert(entity, component);
	}

	void ComponentRegistry::add_component(Entity entity, ComponentType component_type) {
		const char* type_name = m_ComponentTypeNameMap[component_type];
		stl::shared_ptr<CustomComponentList>& component_list = m_CustomComponentLists[type_name];
		if (!component_list)
			component_list = stl::make_shared<CustomComponentList>(mem::ENUM::Game_Components);
		stl::shared_ptr<IComponent> component;
		component_list->default_component->copy(component);
		component_list->insert(entity, component);
	}

	void ComponentRegistry::remove_component(Entity entity, stl::shared_ptr<IComponent> component) {
		const char* type_name = m_ComponentTypeNameMap[component->component_type()];
		stl::shared_ptr<CustomComponentList>& component_list = m_CustomComponentLists[type_name];
		component_list->remove(entity);
	}

	stl::shared_ptr<IComponent> ComponentRegistry::component(Entity entity, ComponentType type) {
		const char* type_name = m_ComponentTypeNameMap[type];
		stl::shared_ptr<CustomComponentList>& component_list = m_CustomComponentLists[type_name];
		return component_list->get(entity);
	}

	stl::shared_ptr<IComponent> ComponentRegistry::component(Entity entity, const char* type_name) {
		stl::shared_ptr<CustomComponentList>& component_list = m_CustomComponentLists[type_name];
		return component_list->get(entity);
	}

	stl::vector<stl::shared_ptr<IComponent>> ComponentRegistry::components(Entity entity, Signature signature) {
		stl::vector<stl::shared_ptr<IComponent>> return_vector{};
		for (int i = 0; i < signature.size(); ++i) {
			if (!signature[i])
				continue;
			const char* component_name = m_ComponentTypeNameMap[static_cast<ComponentType>(i)];
			stl::shared_ptr<CustomComponentList> component_list = m_CustomComponentLists[component_name];
			if (component_list)
				return_vector.push_back(component_list->get(entity));
		}
		return return_vector;
	}

	void ComponentRegistry::entity_destroyed(Entity entity) {
		for (auto& [name, component_list] : m_EngineComponentLists) {
			component_list->entity_destroyed(entity);
		}
		for (auto& [_, component_list] : m_CustomComponentLists) {
			component_list->entity_destroyed(entity);
		}
	}
} // namespace Sapfire::components
