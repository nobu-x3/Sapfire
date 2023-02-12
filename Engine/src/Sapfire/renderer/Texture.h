#pragma once

#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual void bind() = 0;
		virtual int get_width() const = 0;
		virtual int get_height() const = 0;
		virtual RendererID get_id() const = 0;
		virtual bool is_loaded() const = 0;

		static Ref<Texture> create(const std::string& path);
	};
}