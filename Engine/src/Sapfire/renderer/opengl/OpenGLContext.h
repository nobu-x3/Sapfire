#pragma once

#include "Sapfire/renderer/RenderingContext.h"
#include <glfw/glfw3.h>

class OpenGLContext : public RenderingContext
{
public:
	OpenGLContext(GLFWwindow* windowHandle);
	virtual ~OpenGLContext() {}
	virtual void Init() override;
	virtual void SwapBuffers() override;
	virtual void Shutdown() override;

private:
	GLFWwindow* mWindowHandle;
};
