#include "engpch.h"
#include "Texture.h"
#include "renderer/RendererAPI.h"
#include "renderer/opengl/OpenGLTexture.h"

Ref<Texture> Texture::Create(const std::string &path)
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
