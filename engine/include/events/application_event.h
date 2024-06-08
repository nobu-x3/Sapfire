#pragma once

#include "events/event.h"

namespace Sapfire {

	class WindowResizeEvent final : public Event {
	public:
		WindowResizeEvent(u64 width, u64 height) : m_Width(width), m_Height(height) {}
		u64 width() const { return m_Width; }
		u64 height() const { return m_Height; }
		stl::string to_string() const final {
			stl::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
	private:
		u64 m_Width, m_Height;
	};

	class WindowResizeFinishedEvent final : public Event {
	public:
		WindowResizeFinishedEvent() = default;
		EVENT_CLASS_TYPE(WindowResizeFinished);
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
	};

	class WindowCloseEvent final : public Event {
	public:
		WindowCloseEvent() = default;
		EVENT_CLASS_TYPE(WindowClose);
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
	};
} // namespace Sapfire
