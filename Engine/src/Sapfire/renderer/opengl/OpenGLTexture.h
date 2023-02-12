#pragma once
#include "Sapfire/renderer/Texture.h"
#include <glad/glad.h>

namespace Sapfire
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const std::string& fileName);
		~OpenGLTexture();
		virtual void bind() override;
		virtual int get_width() const override { return mWidth; }
		virtual int get_height() const override { return mHeight; }
		virtual RendererID get_id() const override { return mTextureID; }
		virtual bool is_loaded() const override { return mIsLoaded; }
	private:
		// OpenGL id
		RendererID mTextureID;
		bool mIsLoaded = false;
		int mWidth;
		int mHeight;
		GLenum mInternalFormat, mDataFormat;
	};
}