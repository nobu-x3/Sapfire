#pragma once

#include "component.h"

namespace Sapfire::components {
	class AnimComponent {
		RTTI;
		ENGINE_COMPONENT(AnimComponent);

	public:
		AnimComponent();
		AnimComponent(const AnimComponent&);
		AnimComponent(AnimComponent&&) noexcept;
		AnimComponent& operator=(const AnimComponent&);
		AnimComponent& operator=(AnimComponent&&) noexcept;

		inline void time_pos(f32 time) { m_TimePos = time; }
		inline f32 time_pos() const { return m_TimePos; }
		inline UUID current_clip() const { return m_CurrentClip; }
		inline void current_clip(UUID uuid) { m_CurrentClip = uuid; }
		inline UUID skinned_data_uuid() const { return m_SkinnedDataUUID; }
		inline void skinned_data_uuid(UUID uuid) { m_SkinnedDataUUID = uuid; }
		inline UUID uuid() const { return m_ComponentUUID; }

	private:
		void register_rtti();

	private:
		UUID m_SkinnedDataUUID{0};
		f32 m_TimePos{0};
		UUID m_CurrentClip{0};
		UUID m_ComponentUUID{};
	};
} // namespace Sapfire::components