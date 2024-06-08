#pragma once

#include "components/component.h"
#include "core/core.h"
#include "core/rtti.h"

namespace Sapfire::components {

	class TestCustomComponent : public IComponent {
		RTTI_COMPONENT;
		COMPONENT(TestCustomComponent)

	public:
		TestCustomComponent();
		TestCustomComponent(stl::string_view message);
		TestCustomComponent(const TestCustomComponent&);
		TestCustomComponent(TestCustomComponent&&) noexcept;
		TestCustomComponent& operator=(const TestCustomComponent&);
		TestCustomComponent& operator=(TestCustomComponent&&) noexcept;
		void register_rtti();
		void update(f32 delta_time) override;
		void inline message(stl::string_view msg) { m_Message = msg; }

	private:
		stl::string m_Message{"Message"};
	};
} // namespace Sapfire
