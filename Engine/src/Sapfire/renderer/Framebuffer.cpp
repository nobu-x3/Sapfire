#include "engpch.h"
#include "Framebuffer.h"
#include "RendererAPI.h"
#include "opengl/OpenGLFramebuffer.h"

namespace Sapfire
{
	Ref<Framebuffer> Framebuffer::create(const FramebufferProperties& props)
	{
		auto api = RendererAPI::get_api();
		switch(api)
		{
		case RendererAPI::API::OpenGL:
			return create_ref<OpenGLFramebuffer>(props);
		default:
			ENGINE_ERROR("API not implemented.");
			return nullptr;
		}
	}
}