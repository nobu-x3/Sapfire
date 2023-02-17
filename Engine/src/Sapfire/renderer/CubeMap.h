#pragma once
#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class CubeMap
	{
	public:
		virtual ~CubeMap() = default;
		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual const RendererID& get_id() const { return mRendererID; }
		virtual uint32_t get_width() = 0;
		virtual uint32_t get_height() = 0;
		virtual bool is_loaded() = 0;
		static Ref<CubeMap> create(std::array<std::string, 6> textureFaces);
	protected:
		RendererID mRendererID;
	};
}
