#pragma once

#include "Sapfire/renderer/Framebuffer.h"

namespace Sapfire
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferProperties& props);
		virtual ~OpenGLFramebuffer();
		inline virtual const
			FramebufferProperties& GetPropeties() const override { return mProperties; }
		inline virtual const RendererID& GetColorAttachmentRendererID() const override { return mColorAttachment; }
		inline virtual const RendererID& GetDepthAttachmentRendererID() const override { return mDepthAttachment; }
		inline virtual const RendererID& GetRendererID() const override { return mRendererID; }
		virtual void Invalidate() override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(uint16_t width, uint16_t height) override;

	private:
		RendererID mRendererID = 0;
		RendererID mColorAttachment = 0, mDepthAttachment = 0;
		FramebufferProperties mProperties;
	};

}
