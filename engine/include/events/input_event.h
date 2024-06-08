#pragma once

#include "core/core.h"
#include "events/event.h"

namespace Sapfire {

	enum class MouseButton { LMB, RMB, MMB };

	class MouseButtonEvent final : public Event {
	public:
		MouseButtonEvent(MouseButton button, bool isDown) : m_Button(button), m_IsDown(isDown) {}
		bool is_down() const { return m_IsDown; }
		MouseButton button() const { return m_Button; }

		stl::string to_string() const final {
			stl::stringstream ss;
			ss << "MouseButtonEvent: ";
			switch (m_Button) {
			case MouseButton::RMB:
				ss << "RMB";
				break;
			case MouseButton::MMB:
				ss << "MMB";
				break;
			default:
				ss << "LMB";
				break;
			}
			ss << ", is down: " << m_IsDown;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButton)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryMouseButton)
	private:
		MouseButton m_Button;
		bool m_IsDown;
	};

	class MouseMovedEvent final : public Event {
	public:
		MouseMovedEvent(s32 new_x, s32 new_y) : m_X(new_x), m_Y(new_y) {}
		s32 x() const { return m_X; }
		s32 y() const { return m_Y; }
		stl::string to_string() const final {
			stl::stringstream ss;
			ss << "MouseMovedEvent: delta x: " << m_X << ", delta y: " << m_Y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryMouse)
	private:
		s32 m_X;
		s32 m_Y;
	};
} // namespace Sapfire
