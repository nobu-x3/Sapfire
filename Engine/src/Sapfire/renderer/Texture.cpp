#include "engpch.h"
#include "Texture.h"
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLTexture.h"

namespace Sapfire
{
	Ref<Texture> Texture::create(const std::string& path)
	{
		switch (RendererAPI::get_api())
		{
		case RendererAPI::API::OpenGL:
			return create_ref<OpenGLTexture>(path);
		default:
			ENGINE_ERROR("Unknown RendererAPI!");
			return nullptr;
		}
	}
}