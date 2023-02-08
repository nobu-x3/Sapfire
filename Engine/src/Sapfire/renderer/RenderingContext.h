#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	class RenderingContext
	{
	public:
		virtual ~RenderingContext() {}
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void Shutdown() = 0;
		static Scope<RenderingContext> Create(void* window);
	};
}