#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void init() override;
		virtual void clear_screen() override;
		virtual void set_clear_color(glm::vec4& color) override;
		virtual void draw(const Ref<VertexArray>& vertexArray) override;
		virtual void draw_mesh() override;
		virtual void draw(size_t count) override;
		virtual void set_viewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		virtual void disable_depth() override;
		virtual void disable_culling() override;
		virtual void enable_culling() override;
		virtual void enable_depth() override;
	};
}