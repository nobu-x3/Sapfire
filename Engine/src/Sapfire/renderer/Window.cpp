#include "engpch.h"
#include "Window.h"
#include "Sapfire/renderer/Renderer.h"
#include "Sapfire/renderer/SDL/SDLWindow.h"
#include "Sapfire/renderer/GLFW/GLFWWindow.h"

namespace Sapfire
{
	Window* Window::create(const WindowProperties& props)
	{
		auto api = Renderer::get_window_api();
		switch (api)
		{
			/*case WindowAPI::SDL:
				return new SDLWindow(props);*/
		case WindowAPI::GLFW:
			return new GLFWWindow(props);
		default:
			return nullptr;
		}
	}
}