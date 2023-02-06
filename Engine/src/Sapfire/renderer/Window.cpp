#include "engpch.h"
#include "Window.h"
#include "Sapfire/renderer/Renderer.h"
#include "Sapfire/renderer/SDL/SDLWindow.h"
#include "Sapfire/renderer/GLFW/GLFWWindow.h"

Window *Window::Create(const WindowProperties &props)
{
	auto api = Renderer::GetWindowAPI();
	switch (api)
	{
	/*case WindowAPI::SDL:
		return new SDLWindow(props);*/
	case WindowAPI::GLFW:
		return new GLFWWindow(props);
	default:
		return nullptr;
	}
	return nullptr;
}
