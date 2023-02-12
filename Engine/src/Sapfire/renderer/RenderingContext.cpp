#include "engpch.h"
#include "RenderingContext.h"
#include "Sapfire/renderer/opengl/OpenGLContext.h"
#include <glfw/glfw3.h>

namespace Sapfire
{
	Scope<RenderingContext> RenderingContext::create(void* window)
	{
		switch (RendererAPI::get_api())
		{
		case RendererAPI::API::None: ENGINE_ERROR("Rendering API currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return create_scope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}
		ENGINE_ERROR("Unsupported API!");
		return nullptr;
	}
}