#include "engpch.h"
#include "GLFWInput.h"
#include "Sapfire/core/Application.h"
#include "Sapfire/renderer/Window.h"

namespace Sapfire
{
	Input* Input::sInstance = new GLFWInput();

	bool GLFWInput::KeyPressed_Impl(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, (int)keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool GLFWInput::KeyReleased_Impl(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, (int)keycode);
		return state == GLFW_RELEASE;
	}

	bool GLFWInput::MouseButtonPressed_Impl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	bool GLFWInput::MouseButtonDown_Impl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	bool GLFWInput::MouseButtonUp_Impl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	std::pair<float, float> GLFWInput::GetMousePosition_Impl()
	{
		auto window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}

	float GLFWInput::GetMouseX_Impl()
	{
		auto [x, y] = GetMousePosition_Impl();
		return x;
	}

	float GLFWInput::GetMouseY_Impl()
	{
		auto [x, y] = GetMousePosition_Impl();
		return y;
	}
}