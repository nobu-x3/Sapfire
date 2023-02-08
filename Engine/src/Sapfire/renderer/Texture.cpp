#include "engpch.h"
#include "Texture.h"
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLTexture.h"

namespace Sapfire
{
	Ref<Texture> Texture::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture>(path);
		default:
			ENGINE_ERROR("Unknown RendererAPI!");
			return nullptr;
		}
		return nullptr;
	}
}