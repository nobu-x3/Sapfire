#include "Texture.h"
#include "GL/glew.h"
#include "SOIL2.h"
#include <SDL_log.h>
bool Texture::Load(const std::string &fileName)
{
	int channels = 0;

	unsigned char *image = SOIL_load_image(fileName.c_str(), &mWidth, &mHeight, &channels, SOIL_LOAD_AUTO);

	if (image == nullptr)
	{
		SDL_Log("SOIL failed to load image %s: %s", fileName.c_str(), SOIL_last_result());
		return false;
	}

	int format = GL_RGB;
	if (channels == 4)
	{
		format = GL_RGBA;
	}

	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);

	// Enable bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
	/* int channels = 0; */
	/* unsigned char *img = SOIL_load_image(fileName.c_str(), &mWidth, &mHeight, &channels, SOIL_LOAD_AUTO); */
	/* if (img == nullptr) */
	/* { */
	/* 	SDL_Log("TEXTURE::Failed to load texture at %s : %s", fileName.c_str(), SOIL_last_result()); */
	/* 	return false; */
	/* } */
	/* int format = channels == 4 ? GL_RGBA : GL_RGB; */
	/* glGenTextures(1, &mTextureID); */
	/* glBindTexture(GL_TEXTURE_2D, mTextureID); */
	/* glTexImage2D(GL_TEXTURE_2D,    // Texture target */
	/* 	     0,		       // LOD */
	/* 	     format,	       // Color format OpenGL will use */
	/* 	     mWidth,	       // Width of texture */
	/* 	     mHeight,	       // height of texture */
	/* 	     0,		       // border - always 0 */
	/* 	     format,	       // color format for input data */
	/* 	     GL_UNSIGNED_BYTE, // bit depth of input data, ubyte specifies 8-bit channel, */
	/* 	     img	       // pointer to image data */
	/* ); */
	/* SOIL_free_image_data(img); */
	/* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */
	/* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
	/* return true; */
}

void Texture::Unload()
{
	glDeleteTextures(1, &mTextureID);
}

void Texture::SetActive()
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}
