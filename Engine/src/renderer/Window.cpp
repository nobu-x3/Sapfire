#include "engpch.h"
#include "Window.h"
#include "renderer/Renderer.h"
#include "renderer/SDL/SDLWindow.h"

Window *Window::Create(const WindowProperties &props)
{
	auto api = Renderer::GetWindowAPI();
	switch (api)
	{
	case WindowAPI::SDL:
		return new SDLWindow(props);
	default:
		return nullptr;
	}
	return nullptr;
}
