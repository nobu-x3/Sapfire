#include "engpch.h"
#include "Sapfire/core/Input.h"
#include "Sapfire/core/Application.h"
#include "Sapfire/renderer/Window.h"
#include <GLFW/glfw3.h>

namespace Sapfire
{
	bool Input::key_pressed(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::get_instance().get_window().get_native_window());
		auto state = glfwGetKey(window, static_cast<int>(keycode));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::key_released(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::get_instance().get_window().get_native_window());
		auto state = glfwGetKey(window, static_cast<int>(keycode));
		return state == GLFW_RELEASE;
	}

	bool Input::mouse_button_down(MouseButton button)
	{
		auto window = static_cast<GLFWwindow*>(Application::get_instance().get_window().get_native_window());
		auto state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_PRESS;
	}

	bool Input::mouse_button_up(MouseButton button)
	{
		auto window = static_cast<GLFWwindow*>(Application::get_instance().get_window().get_native_window());
		auto state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_RELEASE;
	}

	std::pair<float, float> Input::get_mouse_position()
	{
		auto window = static_cast<GLFWwindow*>(Application::get_instance().get_window().get_native_window());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { static_cast<float>(xPos), static_cast<float>(yPos) };
	}

	float Input::get_mouse_x()
	{
		auto [x, y] = get_mouse_position();
		return x;
	}

	float Input::get_mouse_y()
	{
		auto [x, y] = get_mouse_position();
		return y;
	}
}