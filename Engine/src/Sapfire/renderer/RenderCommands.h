#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	class RenderCommands
	{
	public:
		static void init() { sRendererAPI->init(); }

		static void set_viewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
		{
			sRendererAPI->set_viewport(x, y, width, height);
		}

		static void draw(const Ref<VertexArray>& vertexArray) { sRendererAPI->draw(vertexArray); }
		static void draw(size_t count) { sRendererAPI->draw(count); }
		static void draw_mesh() { sRendererAPI->draw_mesh(); }
		static void draw_skybox(const Ref<VertexBuffer>& VB, const Ref<CubeMap>& cubemap) { sRendererAPI->draw_skybox(VB, cubemap); }
		static void set_clear_color(glm::vec4& color) { sRendererAPI->set_clear_color(color); }
		static void clear_screen() { sRendererAPI->clear_screen(); }
		static void disable_culling() { sRendererAPI->disable_culling(); }
		static void disable_depth() { sRendererAPI->disable_depth(); }
		static void enable_culling() { sRendererAPI->enable_culling(); }
		static void enable_depth() { sRendererAPI->enable_depth(); }
		

	private:
		static RendererAPI* sRendererAPI;
	};
}
