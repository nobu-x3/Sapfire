#pragma once
#include "Sapfire/renderer/VertexArray.h"

namespace Sapfire
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1
		};

	public:
		virtual void init() = 0;
		virtual void clear_screen() = 0;
		virtual void set_clear_color(glm::vec4& color) = 0;
		virtual void set_viewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
		virtual void draw(const Ref<VertexArray>& vertexArray) = 0;
		virtual void draw(size_t count) = 0;
		virtual void draw_mesh() = 0;
		static API get_api() { return sAPI; }

	private:
		static API sAPI;
	};
}