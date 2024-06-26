#pragma once

#include "components/entity.h"
#include "core/memory.h"
#include "core/rtti.h"
#include "core/stl/shared_ptr.h"

namespace Sapfire::components {

	using ComponentType = u8;

	class IComponent {
	public:
		virtual ~IComponent() = default;
		virtual void update(f32 delta_time) = 0;
		virtual const stl::string& to_string() const = 0;
		virtual rtti::rtti_object& get_rtti() = 0;
		virtual ComponentType component_type() = 0;
		virtual void copy(stl::shared_ptr<IComponent>&) = 0;
	};

	class IComponentList {
	public:
		virtual ~IComponentList() = default;
		virtual void entity_destroyed(Entity entity) = 0;
		virtual stl::string to_string() = 0;
	};

	template <typename T>
	class EngineComponentList : public IComponentList {
	public:
		stl::string to_string() override { return T::to_string(); }

		void insert(Entity entity, T component) {
			if (m_EntityToIndexMap.contains(entity)) {
				auto index = m_EntityToIndexMap[entity];
				m_Components[index] = component;
				/* remove(entity); */
				return;
			}
			m_EntityToIndexMap[entity] = m_Components.size();
			m_IndexToEntityMap[m_Components.size()] = entity;
			m_Components.push_back(component);
		}

		bool exists(Entity entity) { return m_EntityToIndexMap.contains(entity); }

		void remove(Entity entity) {
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

		T& get(Entity entity) { return m_Components[m_EntityToIndexMap[entity]]; }

		void entity_destroyed(Entity entity) override {
			if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()) {
				remove(entity);
			}
		}

		stl::vector<T>& components() { return m_Components; }

	private:
		stl::vector<T> m_Components;
		stl::unordered_map<Entity, size_t> m_EntityToIndexMap;
		stl::unordered_map<size_t, Entity> m_IndexToEntityMap;
	};

	class CustomComponentList : public IComponentList {

	public:
		CustomComponentList() = default;
		CustomComponentList(const stl::shared_ptr<IComponent>& def_comp);
		CustomComponentList(const CustomComponentList&) = default;
		CustomComponentList(CustomComponentList&&) noexcept = default;
		CustomComponentList& operator=(const CustomComponentList&) = default;
		CustomComponentList& operator=(CustomComponentList&&) noexcept = default;
		stl::shared_ptr<IComponent> default_component;
		stl::string to_string() override;
		void insert(Entity entity, stl::shared_ptr<IComponent> component);
		void remove(Entity entity);
		stl::shared_ptr<IComponent> get(Entity entity);
		void entity_destroyed(Entity entity) override;
		const stl::vector<stl::shared_ptr<IComponent>>& components() const { return m_Components; }

	private:
		stl::vector<stl::shared_ptr<IComponent>> m_Components{};
		stl::unordered_map<Entity, size_t> m_EntityToIndexMap{};
		stl::unordered_map<size_t, Entity> m_IndexToEntityMap{};
	};

	class ComponentRegistry {

	public:
		template <typename T>
		static void global_register_engine_component() {
			const char* type_name = typeid(T).name();
			s_ComponentTypes[type_name] = s_NextComponentTypeNumber;
			s_ComponentTypeNameMap[s_NextComponentTypeNumber] = type_name;
			s_EngineComponentLists[type_name] = stl::make_shared<EngineComponentList<T>>(mem::ENUM::Engine_Components);
			s_NextComponentTypeNumber++;
		}

		static void global_register_custom_component(stl::shared_ptr<IComponent> component) {
			auto type_str = component->to_string();
			const char* type_name = type_str.c_str();
			s_ComponentTypes[type_name] = s_NextComponentTypeNumber;
			s_ComponentTypeNameMap[s_NextComponentTypeNumber] = type_name;
			s_CustomComponentLists[type_name] = stl::make_shared<CustomComponentList>(mem::ENUM::Game_Components, component);
			s_NextComponentTypeNumber++;
		}

	public:
		ComponentRegistry();

	public:
		template <typename T>
		void register_engine_component() {
			const char* type_name = typeid(T).name();
			m_ComponentTypes[type_name] = m_NextComponentTypeNumber;
			m_ComponentTypeNameMap[m_NextComponentTypeNumber] = type_name;
			m_EngineComponentLists[type_name] = stl::make_shared<EngineComponentList<T>>(mem::ENUM::Engine_Components);
			m_NextComponentTypeNumber++;
		}

		template <typename T>
		ComponentType component_type() {
			const char* type_name = typeid(T).name();
			return m_ComponentTypes[type_name];
		}

		template <typename T>
		bool has_engine_component(Entity entity) {
			const char* type_name = typeid(T).name();
			return m_EngineComponentLists.contains(type_name) &&
				std::static_pointer_cast<EngineComponentList<T>>(m_EngineComponentLists[type_name])->exists(entity);
		}

		template <typename T>
		void add_engine_component(Entity entity, T component) {
			const char* type_name = typeid(T).name();
			std::static_pointer_cast<EngineComponentList<T>>(m_EngineComponentLists[type_name])->insert(entity, component);
		}

		template <typename T>
		void remove_engine_component(Entity entity) {
			const char* type_name = typeid(T).name();
			std::static_pointer_cast<EngineComponentList<T>>(m_EngineComponentLists[type_name])->remove(entity);
		}

		template <typename T>
		T& get_engine_component(Entity entity) {
			const char* type_name = typeid(T).name();
			return std::static_pointer_cast<EngineComponentList<T>>(m_EngineComponentLists[type_name])->get(entity);
		}

		template <typename T>
		stl::vector<T>& engine_components() {
			const char* type_name = typeid(T).name();
			return std::static_pointer_cast<EngineComponentList<T>>(m_EngineComponentLists[type_name])->components();
		}

		void add_component(Entity entity, stl::shared_ptr<IComponent> component);
		void add_component(Entity entity, ComponentType component_type);
		void remove_component(Entity entity, stl::shared_ptr<IComponent> component);
		stl::shared_ptr<IComponent> component(Entity entity, ComponentType type);
		stl::shared_ptr<IComponent> component(Entity entity, const char* type_name);
		stl::vector<stl::shared_ptr<IComponent>> components(Entity entity, Signature signature);
		void entity_destroyed(Entity entity);

	private:
		stl::unordered_map<const char*, ComponentType> m_ComponentTypes{};
		stl::unordered_map<ComponentType, const char*> m_ComponentTypeNameMap{};
		stl::unordered_map<const char*, stl::shared_ptr<IComponentList>> m_EngineComponentLists{};
		stl::unordered_map<const char*, stl::shared_ptr<CustomComponentList>> m_CustomComponentLists{};
		ComponentType m_NextComponentTypeNumber{};

	public:
		static stl::unordered_map<const char*, ComponentType> s_ComponentTypes;
		static stl::unordered_map<ComponentType, const char*> s_ComponentTypeNameMap;
		static stl::unordered_map<const char*, stl::shared_ptr<IComponentList>> s_EngineComponentLists;
		static stl::unordered_map<const char*, stl::shared_ptr<CustomComponentList>> s_CustomComponentLists;
		static ComponentType s_NextComponentTypeNumber;
	};

#define COMPONENT(type)                                                                                                                    \
public:                                                                                                                                    \
	inline const ::Sapfire::stl::string& to_string() const override { return s_ComponentName; }                                            \
	inline ::Sapfire::components::ComponentType component_type() override { return s_ComponentType; }                                      \
	inline void copy(::Sapfire::stl::shared_ptr<IComponent>& dest) override {                                                              \
		dest = ::Sapfire::stl::make_shared<type>(::Sapfire::mem::ENUM::Game_Components, *this);                                            \
	}                                                                                                                                      \
                                                                                                                                           \
private:                                                                                                                                   \
	static ::Sapfire::stl::string s_ComponentName;                                                                                         \
	static ::Sapfire::components::ComponentType s_ComponentType;

#define COMPONENT_IMPL(type)                                                                                                               \
	::Sapfire::stl::string type::s_ComponentName = #type;                                                                                  \
	::Sapfire::components::ComponentType type::s_ComponentType = ::Sapfire::components::ComponentRegistry::s_NextComponentTypeNumber;      \
	::Sapfire::stl::shared_ptr<type> default_component_##type = ::Sapfire::stl::make_shared<type>(::Sapfire::mem::ENUM::Game_Components);  \
	struct RegisteredComponent##type {                                                                                                     \
		RegisteredComponent##type() {                                                                                                      \
			::Sapfire::components::ComponentRegistry::global_register_custom_component(default_component_##type);                          \
		}                                                                                                                                  \
	};                                                                                                                                     \
	RegisteredComponent##type _registered_component;

#define ENGINE_COMPONENT_IMPL(type)                                                                                                        \
	struct RegisteredComponent##type {                                                                                                     \
		RegisteredComponent##type() { ::Sapfire::components::ComponentRegistry::global_register_engine_component<type>(); }                \
	};                                                                                                                                     \
	RegisteredComponent##type _registered_component;

#define ENGINE_COMPONENT(type)                                                                                                             \
public:                                                                                                                                    \
	static ::Sapfire::stl::string to_string() { return #type; }                                                                            \
                                                                                                                                           \
private:
} // namespace Sapfire::components
