#pragma once

#include "Sapfire/renderer/RenderingContext.h"
#include <glfw/glfw3.h>

namespace Sapfire
{
	class OpenGLContext : public RenderingContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual ~OpenGLContext() {}
		virtual void init() override;
		virtual void swap_buffers() override;
		virtual void shutdown() override;

	private:
		GLFWwindow* mWindowHandle;
	};
}