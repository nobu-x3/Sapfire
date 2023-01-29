#include "OpenGLTexture.h"
#include "engine/Log.h"
#include "engine/engpch.h"
#include <GL/glew.h>
#include <SDL2/SDL_log.h>
#include <SOIL2.h>

OpenGLTexture::OpenGLTexture(const std::string &fileName)
{
	int channels = 0;
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

OpenGLTexture::~OpenGLTexture()
{
	glDeleteTextures(1, &mTextureID);
}

void OpenGLTexture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}
