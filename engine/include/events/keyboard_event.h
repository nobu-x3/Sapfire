#pragma once

#include "core/core.h"
#include "events/event.h"

namespace Sapfire {

	class KeyPressedEvent final : public Event {
	public:
		KeyPressedEvent(u64 param) : m_State(param) {}
		u64 state() const { return m_State; }
		EVENT_CLASS_TYPE(KeyPressed)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryKeyboard)
	private:
		u64 m_State;
	};

	class KeyReleasedEvent final : public Event {
	public:
		KeyReleasedEvent(u64 param) : m_State(param) {}
		u64 state() const { return m_State; }
		EVENT_CLASS_TYPE(KeyReleased)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryKeyboard)
	private:
		u64 m_State;
	};
} // namespace Sapfire
