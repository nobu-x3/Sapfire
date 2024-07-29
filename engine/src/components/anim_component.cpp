#include "engpch.h"

#include "components/anim_component.h"

namespace Sapfire::components {
	ENGINE_COMPONENT_IMPL(AnimComponent);

	AnimComponent::AnimComponent() { register_rtti(); }

	AnimComponent::AnimComponent(const AnimComponent& other) {
		m_SkinnedDataUUID = other.m_SkinnedDataUUID;
		m_TimePos = other.m_TimePos;
		m_CurrentClip = other.m_CurrentClip;
		m_ComponentUUID = other.m_ComponentUUID;
		register_rtti();
	}

	AnimComponent::AnimComponent(AnimComponent&& other) noexcept {
		m_SkinnedDataUUID = std::move(other.m_SkinnedDataUUID);
		m_TimePos = std::move(other.m_TimePos);
		m_CurrentClip = std::move(other.m_CurrentClip);
		m_ComponentUUID = std::move(other.m_ComponentUUID);
		register_rtti();
	}

	AnimComponent& AnimComponent::operator=(const AnimComponent& other) {
		m_SkinnedDataUUID = other.m_SkinnedDataUUID;
		m_TimePos = other.m_TimePos;
		m_CurrentClip = other.m_CurrentClip;
		m_ComponentUUID = other.m_ComponentUUID;
		register_rtti();
		return *this;
	}

	AnimComponent& AnimComponent::operator=(AnimComponent&& other) noexcept {
		m_SkinnedDataUUID = std::move(other.m_SkinnedDataUUID);
		m_TimePos = std::move(other.m_TimePos);
		m_CurrentClip = std::move(other.m_CurrentClip);
		m_ComponentUUID = std::move(other.m_ComponentUUID);
		register_rtti();
		return *this;
	}

	void AnimComponent::register_rtti() {
		BEGIN_RTTI();
		ADD_RTTI_REFERENCE(rtti::rtti_reference_type::SkinnedData, "Skinned Data", &m_SkinnedDataUUID, nullptr);
		END_RTTI();
	}

} // namespace Sapfire::components