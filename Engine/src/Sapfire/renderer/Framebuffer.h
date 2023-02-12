#pragma once
#include "Sapfire/core/Core.h"

namespace Sapfire
{
	enum class FramebufferFormat
	{
		None = 0,
		RGBA8 = 1,
		RGBA16F = 2
	};

	struct FramebufferProperties
	{
		uint16_t Width, Height;
		FramebufferFormat Format;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;
		virtual const FramebufferProperties& get_propeties() const = 0;
		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void invalidate() = 0;
		virtual void resize(uint16_t width, uint16_t height) = 0;
		virtual const RendererID& get_color_attachment_renderer_id() const = 0;
		virtual const RendererID& get_depth_attachment_renderer_id() const = 0;
		virtual const RendererID& get_renderer_id() const = 0;

	public:
		static Ref<Framebuffer> create(const FramebufferProperties& props);
	};
}