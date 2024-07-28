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

	private:
		void register_rtti();

	private:
		UUID m_SkinnedDataUUID{0};
	};
} // namespace Sapfire::components