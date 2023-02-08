#pragma once

#include "engpch.h"
#include "Sapfire/core/Core.h"
#include "Sapfire/events/Event.h"

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
		virtual ~Window() {}
		virtual void OnUpdate() = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual void SetEventCallback(const EventCallback& event) = 0;
		virtual float GetTime() const = 0;
		virtual void* GetNativeWindow() const = 0;
		static Window* Create(const WindowProperties& properties = WindowProperties());
	};
}