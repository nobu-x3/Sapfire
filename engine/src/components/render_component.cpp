#include "engpch.h"

#include "components/render_component.h"
#include "core/rtti.h"

namespace Sapfire::components {
	ENGINE_COMPONENT_IMPL(RenderComponent);

	RenderComponent::RenderComponent(UUID mesh_uuid, CPUData cpu_data, PerDrawConstants per_draw_constants,
									 stl::function<void(RenderComponent*)> optional_setter) :
		m_MeshUUID(mesh_uuid), m_CPUData(cpu_data), m_PerDrawConstants(per_draw_constants), m_OptionalSetter(optional_setter) {
		register_rtti();
	}

	RenderComponent::RenderComponent() { register_rtti(); }

	RenderComponent::RenderComponent(const RenderComponent& other) :
		m_MeshUUID(other.m_MeshUUID), m_MaterialUUID(other.m_MaterialUUID), m_TextureUUID(other.m_TextureUUID), m_CPUData(other.m_CPUData),
		m_PerDrawConstants(other.m_PerDrawConstants), m_OptionalSetter(other.m_OptionalSetter) {
		register_rtti();
	}

	RenderComponent::RenderComponent(RenderComponent&& other) :
		m_MeshUUID(std::move(other.m_MeshUUID)), m_MaterialUUID(std::move(other.m_MaterialUUID)),
		m_TextureUUID(std::move(other.m_TextureUUID)), m_CPUData(std::move(other.m_CPUData)),
		m_PerDrawConstants(std::move(other.m_PerDrawConstants)), m_OptionalSetter(std::move(other.m_OptionalSetter)) {
		register_rtti();
	}

	RenderComponent& RenderComponent::operator=(const RenderComponent& other) {
		m_MeshUUID = other.m_MeshUUID;
		m_MaterialUUID = other.m_MaterialUUID;
		m_TextureUUID = other.m_TextureUUID;
		m_PerDrawConstants = other.m_PerDrawConstants;
		m_CPUData = other.m_CPUData;
		m_OptionalSetter = other.m_OptionalSetter;
		register_rtti();
		return *this;
	}

	RenderComponent& RenderComponent::operator=(RenderComponent&& other) {
		m_MeshUUID = std::move(other.m_MeshUUID);
		m_MaterialUUID = std::move(other.m_MaterialUUID);
		m_TextureUUID = std::move(other.m_TextureUUID);
		m_PerDrawConstants = std::move(other.m_PerDrawConstants);
		m_CPUData = std::move(other.m_CPUData);
		m_OptionalSetter = std::move(other.m_OptionalSetter);
		register_rtti();
		return *this;
	}

	void RenderComponent::register_rtti() {
		BEGIN_RTTI();
		ADD_RTTI_REFERENCE(rtti::rtti_reference_type::Mesh, "Mesh", &m_MeshUUID, [this]() {
			if (m_OptionalSetter)
				m_OptionalSetter(this);
		});
		END_RTTI();
	}
} // namespace Sapfire::components
