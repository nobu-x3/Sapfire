#pragma once

#include "engpch.h"
#include "Sapfire/events/Event.h"
#include "../core/Input.h"

namespace Sapfire
{
	using EventCallbackFn = std::function<void(Event&)>;

	struct WindowData
	{
		std::string Title;
		unsigned int Width, Height;

		EventCallbackFn EventCallback;
	};

	struct WindowProperties
	{
		std::string Title;
		unsigned int Width, Height;
		WindowProperties(const std::string& title = "Engine", unsigned int width = 1280, unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	class Window
	{
	public:
		using EventCallback = std::function<void(Event&)>;
		virtual ~Window() = default;
		virtual void on_update() = 0;
		virtual unsigned int get_width() const = 0;
		virtual unsigned int get_height() const = 0;
		virtual void set_event_callback(const EventCallback& event) = 0;
		virtual float get_time() const = 0;
		virtual void* get_native_window() const = 0;
		static Window* create(const WindowProperties& properties = WindowProperties());
	};
}