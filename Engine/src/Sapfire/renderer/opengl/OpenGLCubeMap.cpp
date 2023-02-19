#include "engpch.h"
#include "OpenGLCubeMap.h"
#include <stb_image.h>

namespace Sapfire
{
	OpenGLCubeMap::OpenGLCubeMap(std::array<std::string, 6> textureFaces) : mWidth(0), mHeight(0)
	{
		PROFILE_FUNCTION();
		glGenTextures(1, &mRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);
		stbi_uc* data = nullptr;
		stbi_set_flip_vertically_on_load(0);
		for (int i = 0; i < 6; ++i)
		{
			int width, height, channels;
			data = stbi_load(textureFaces[i].c_str(), &width, &height, &channels, 0);
			if (i > 0 && (mWidth != width || mHeight != height))
			{
				ENGINE_ERROR(
					"Texture faces in a cube map must be the same size. Texture face '{0}' is of different size.",
					textureFaces[i]);
				stbi_image_free(data);
				shutdown();
				return;
			}
			if (!data)
			{
				ENGINE_ERROR("Failed to load {0}.", textureFaces[i]);
				stbi_image_free(data);
				shutdown();
				return;
			}
			mIsLoaded = true;
			mWidth = width;
			mHeight = height;
			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}
			mInternalFormat = internalFormat;
			mDataFormat = dataFormat;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			             0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLCubeMap::~OpenGLCubeMap()
	{
		shutdown();
	}

	void OpenGLCubeMap::bind()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);
	}

	void OpenGLCubeMap::unbind()
	{
		glBindTextures(0, 1, nullptr);
	}

	void OpenGLCubeMap::shutdown()
	{
		PROFILE_FUNCTION();
		glDeleteTextures(6, &mRendererID);
	}
}
