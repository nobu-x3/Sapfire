#include "engpch.h"
#include "CubeMap.h"
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLCubeMap.h"

namespace Sapfire
{
	///
	/**
	 * \brief
	*	GL_TEXTURE_CUBE_MAP_POSITIVE_X
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 * \param textureFaces 
	 */
	Ref<CubeMap> CubeMap::create(std::array<std::string, 6> textureFaces)
	{
		switch (RendererAPI::get_api())
		{
		case RendererAPI::API::OpenGL:
			return create_ref<OpenGLCubeMap>(textureFaces);
		default:
			ENGINE_ERROR("Unknown RendererAPI!");
			return nullptr;
		}
	}
}