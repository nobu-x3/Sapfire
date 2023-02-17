#include "engpch.h"
#include "OpenGLTexture.h"
#include <stb_image.h>

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
	 * \param fileName 
	 */
	OpenGLTexture::OpenGLTexture(const std::string& fileName)
	{
		PROFILE_FUNCTION();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(fileName.c_str(), &width, &height, &channels, 0);
		}

		if (data)
		{
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
			glCreateTextures(GL_TEXTURE_2D, 1, &mTextureID);
			glTextureParameteri(mTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(mTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureStorage2D(mTextureID, 1, internalFormat, mWidth, mHeight);
			glTextureSubImage2D(mTextureID, 0, 0, 0, mWidth, mHeight, dataFormat, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &mTextureID);
	}

	void OpenGLTexture::bind()
	{
		glBindTextures(0, 1, &mTextureID);
	}
}