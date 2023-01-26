#pragma once

#include "engine/Core.h"
#include "engine/engpch.h"
#include "engine/events/Event.h"

struct WindowProperties
{
	std::string Title;
	unsigned int Width, Height;

	WindowProperties(const std::string &title = "Engine", unsigned int width = 800, unsigned int height = 600)
	    : Title(title), Width(width), Height(height)
	{
	}
};

class Window
{
	public:
	using EventCallback = std::function<void(Event &)>;
	virtual ~Window() {}
	virtual void OnUpdate() = 0;
	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;
	virtual void SetEventCallback(const EventCallback &event) = 0;
	virtual float GetTime() const = 0;
	static Window *Create(const WindowProperties &properties = WindowProperties());
};
