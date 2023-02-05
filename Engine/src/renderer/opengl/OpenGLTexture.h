#pragma once
#include "engine/renderer/Texture.h"

class OpenGLTexture : public Texture
{
	public:
	OpenGLTexture(const std::string &fileName);
	~OpenGLTexture();
	virtual void Bind() override;
	virtual inline int GetWidth() const override { return mWidth; }
	virtual inline int GetHeight() const override { return mHeight; }
	virtual inline RendererID GetID() const override { return mTextureID; }

	private:
	// OpenGL id
	RendererID mTextureID;

	int mWidth;
	int mHeight;
};
