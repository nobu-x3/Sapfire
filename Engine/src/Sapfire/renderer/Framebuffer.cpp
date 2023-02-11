#include "engpch.h"
#include "Framebuffer.h"
#include "RendererAPI.h"
#include "opengl/OpenGLFramebuffer.h"

namespace Sapfire
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferProperties& props)
	{
		auto api = RendererAPI::GetAPI();
		switch(api)
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(props);
		default:
			ENGINE_ERROR("API not implemented.");
			return nullptr;
		}
		ENGINE_ERROR("API not implemented.");
		return nullptr;
	}
}