#include "engpch.h"
#include "OpenGLFramebuffer.h"
#include "glad/glad.h"

namespace Sapfire
{

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferProperties& props) : mProperties(props)
	{
		PROFILE_FUNCTION();
		invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		PROFILE_FUNCTION();
		glDeleteFramebuffers(1, &mRendererID);
		glDeleteTextures(1, &mColorAttachment);
		glDeleteTextures(1, &mDepthAttachment);
	}

	void OpenGLFramebuffer::invalidate()
	{
		PROFILE_FUNCTION();
		if(mRendererID)
		{
			glDeleteFramebuffers(1, &mRendererID);
			glDeleteTextures(1, &mColorAttachment);
			glDeleteTextures(1, &mDepthAttachment);
		}
		glCreateFramebuffers(1, &mRendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
		// color
		glCreateTextures(GL_TEXTURE_2D, 1, &mColorAttachment);
		glBindTexture(GL_TEXTURE_2D, mColorAttachment);
		switch (mProperties.Format)
		{
			case FramebufferFormat::RGBA8:
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mProperties.Width, mProperties.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				break;
			}
			case FramebufferFormat::RGBA16F:
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mProperties.Width, mProperties.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
				break;
			}
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment, 0);
		// depth
		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthAttachment);
		glBindTexture(GL_TEXTURE_2D, mDepthAttachment);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, mProperties.Width, mProperties.Width, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
		);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachment, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			ENGINE_ERROR("Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
		glViewport(0, 0, mProperties.Width, mProperties.Height);
	}

	void OpenGLFramebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::resize(uint16_t width, uint16_t height)
	{
		mProperties.Width = width;
		mProperties.Height = height;
		invalidate();
	}

}
