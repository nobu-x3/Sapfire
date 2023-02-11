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
		virtual ~Framebuffer() {}
		virtual const FramebufferProperties& GetPropeties() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Invalidate() = 0;
		virtual const RendererID& GetColorAttachmentRendererID() const = 0;
		virtual const RendererID& GetDepthAttachmentRendererID() const = 0;
		virtual const RendererID& GetRendererID() const = 0;

	public:
		static Ref<Framebuffer> Create(const FramebufferProperties& props);
	};
}