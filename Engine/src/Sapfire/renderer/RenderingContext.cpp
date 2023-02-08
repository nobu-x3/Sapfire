#include "engpch.h"
#include "RenderingContext.h"
#include "Sapfire/renderer/opengl/OpenGLContext.h"
#include <glfw/glfw3.h>

namespace Sapfire
{
	Scope<RenderingContext> RenderingContext::Create(void* window)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: ENGINE_ERROR("Rendering API currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}
		ENGINE_ERROR("Unsupported API!");
		return nullptr;
	}
}