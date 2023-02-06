#include "engpch.h"
#include "GLFWWindow.h"
#include "Sapfire/Log.h"
#include <Sapfire/events/WindowEvent.h>
#include <Sapfire/events/KeyEvent.h>
#include <Sapfire/events/MouseEvent.h>
static uint8_t sGLFWWindowCount = 0;

static void GLFWErrorCallback(int error, const char* description)
{
	ENGINE_ERROR("GLFW Error ({0}): {1}", error, description);
}

GLFWWindow::GLFWWindow(const WindowProperties& props)
{
	mData.Title = props.Title;
	mData.Width = props.Width;
	mData.Height = props.Height;
	ENGINE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
	if (sGLFWWindowCount == 0)
	{
		int success = glfwInit();
		if (!success)
		{
			ENGINE_ERROR("Could not initialize GLFW!");
		}
		glfwSetErrorCallback(GLFWErrorCallback);
#if defined(DEBUG)
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		}
#endif
		mWindow = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
		++sGLFWWindowCount;
		mContext = RenderingContext::Create(mWindow);
		mContext->Init();
		glfwSetWindowUserPointer(mWindow, &mData);
		// Set GLFW callbacks
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;

		WindowResizeEvent event(width, height);
		data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent event;
		data.EventCallback(event);
			});

		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		switch (action)
		{
		case GLFW_PRESS:
		{
			KeyPressedEvent event(key, 0);
			data.EventCallback(event);
			break;
		}
		case GLFW_RELEASE:
		{
			KeyReleasedEvent event(key);
			data.EventCallback(event);
			break;
		}
		case GLFW_REPEAT:
		{
			KeyPressedEvent event(key, true);
			data.EventCallback(event);
			break;
		}
		}
			});

		glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		KeyReleasedEvent event(keycode);
		data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		switch (action)
		{
		case GLFW_PRESS:
		{
			MouseButtonPressedEvent event(button);
			data.EventCallback(event);
			break;
		}
		case GLFW_RELEASE:
		{
			MouseButtonReleasedEvent event(button);
			data.EventCallback(event);
			break;
		}
		}
			});

		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		MouseScrolledEvent event((float)xOffset, (float)yOffset);
		data.EventCallback(event);
			});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		MouseMovedEvent event((float)xPos, (float)yPos);
		data.EventCallback(event);
			});
	}
}

void GLFWWindow::OnUpdate()
{
}

float GLFWWindow::GetTime() const
{
	return 0.0f;
}
