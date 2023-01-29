#include "Texture.h"
#include "engine/renderer/RendererAPI.h"
#include "engine/renderer/opengl/OpenGLTexture.h"

Ref<Texture> Texture::Create(const std::string &path)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return std::make_shared<OpenGLTexture>(path);
	default:
		ENGINE_ERROR("Unknown RendererAPI!");
		return nullptr;
	}
	return nullptr;
}
