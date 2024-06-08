#include "engpch.h"

#include "components/test_custom_component.h"
#include "core/rtti.h"

namespace Sapfire::components {

	COMPONENT_IMPL(TestCustomComponent)

	TestCustomComponent::TestCustomComponent() { register_rtti(); }

	TestCustomComponent::TestCustomComponent(stl::string_view msg) : m_Message(msg) { register_rtti(); }

	TestCustomComponent::TestCustomComponent(const TestCustomComponent& other) {
		m_Message = other.m_Message;
		register_rtti();
	}

	TestCustomComponent::TestCustomComponent(TestCustomComponent&& other) noexcept {
		m_Message = std::move(other.m_Message);
		register_rtti();
	}

	TestCustomComponent& TestCustomComponent::operator=(const TestCustomComponent& other) {
		m_Message = other.m_Message;
		register_rtti();
		return *this;
	}

	TestCustomComponent& TestCustomComponent::operator=(TestCustomComponent&& other) noexcept {
		m_Message = std::move(other.m_Message);
		register_rtti();
		return *this;
	}

	void TestCustomComponent::update(f32 delta_time) {}

	void TestCustomComponent::register_rtti() {
		BEGIN_RTTI();
		ADD_RTTI_FIELD(rtti::rtti_type::STRING, "Message", &m_Message, nullptr);
		END_RTTI();
	}
} // namespace Sapfire::components
