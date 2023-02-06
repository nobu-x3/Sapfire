#pragma once
#include "Sapfire/renderer/Texture.h"
#include <glad/glad.h>

class OpenGLTexture : public Texture
{
public:
	OpenGLTexture(const std::string& fileName);
	~OpenGLTexture();
	virtual void Bind() override;
	inline virtual int GetWidth() const override { return mWidth; }
	inline virtual int GetHeight() const override { return mHeight; }
	inline virtual RendererID GetID() const override { return mTextureID; }
	inline virtual bool IsLoaded() const override { return mIsLoaded; }
private:
	// OpenGL id
	RendererID mTextureID;
	bool mIsLoaded = false;
	int mWidth;
	int mHeight;
	GLenum mInternalFormat, mDataFormat;
};
