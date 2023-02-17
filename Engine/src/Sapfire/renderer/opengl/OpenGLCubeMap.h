#pragma once

#include <glad/glad.h>
#include "Sapfire/renderer/CubeMap.h"

namespace Sapfire
{
	class OpenGLCubeMap : public CubeMap
	{
	public:
		OpenGLCubeMap(std::array<std::string, 6> textureFaces);
		~OpenGLCubeMap() override;
		virtual void bind() override;
		virtual void unbind() override;
		inline virtual uint32_t get_width() override { return mWidth; }
		inline virtual uint32_t get_height() override { return mHeight; }
		virtual bool is_loaded() override { return mIsLoaded; }

	private:
		void shutdown();
	private:
		uint32_t mWidth, mHeight;
		bool mIsLoaded = false;
		GLenum mInternalFormat, mDataFormat;
	};
}
