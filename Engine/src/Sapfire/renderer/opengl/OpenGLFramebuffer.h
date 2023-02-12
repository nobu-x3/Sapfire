#pragma once

#include "Sapfire/renderer/Framebuffer.h"

namespace Sapfire
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferProperties& props);
		virtual ~OpenGLFramebuffer();

		virtual const
			FramebufferProperties& get_propeties() const override { return mProperties; }

		virtual const RendererID& get_color_attachment_renderer_id() const override { return mColorAttachment; }
		virtual const RendererID& get_depth_attachment_renderer_id() const override { return mDepthAttachment; }
		virtual const RendererID& get_renderer_id() const override { return mRendererID; }
		virtual void invalidate() override;
		virtual void bind() override;
		virtual void unbind() override;
		virtual void resize(uint16_t width, uint16_t height) override;

	private:
		RendererID mRendererID = 0;
		RendererID mColorAttachment = 0, mDepthAttachment = 0;
		FramebufferProperties mProperties;
	};

}
