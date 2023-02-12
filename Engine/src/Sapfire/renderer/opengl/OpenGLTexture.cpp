#include "engpch.h"
#include "OpenGLTexture.h"
#include <stb_image.h>

namespace Sapfire
{
	OpenGLTexture::OpenGLTexture(const std::string& fileName)
	{
		PROFILE_FUNCTION();
		/*int channels = 0;
		unsigned char *image = SOIL_load_image(fileName.c_str(), &mWidth, &mHeight, &channels, SOIL_LOAD_AUTO);
		if (image == nullptr)
		{
			ENGINE_ERROR("SOIL failed to load image {0}: {1}", fileName.c_str(), SOIL_last_result());
			return;
		}
		int format = channels == 4 ? GL_RGBA : GL_RGB;

		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_2D, mTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, image);

		SOIL_free_image_data(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
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

			glGenTextures(1, &mTextureID);
			glBindTexture(GL_TEXTURE_2D, mTextureID);
			//glTextureStorage2D(mTextureID, 1, internalFormat, mWidth, mHeight);

			glTextureParameteri(mTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(mTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			/*glTextureParameteri(mTextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(mTextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

			//glTextureSubImage2D(mTextureID, 0, 0, 0, mWidth, mHeight, dataFormat, GL_UNSIGNED_BYTE, data);
			glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, mWidth, mHeight, 0, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &mTextureID);
	}

	void OpenGLTexture::bind()
	{
		glBindTexture(GL_TEXTURE_2D, mTextureID);
	}
}