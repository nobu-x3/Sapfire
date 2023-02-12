#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	class RenderingContext
	{
	public:
		virtual ~RenderingContext() {}
		virtual void init() = 0;
		virtual void swap_buffers() = 0;
		virtual void shutdown() = 0;
		static Scope<RenderingContext> create(void* window);
	};
}