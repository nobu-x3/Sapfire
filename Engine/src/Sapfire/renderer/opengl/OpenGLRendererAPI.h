#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void ClearScreen() override;
		virtual void SetClearColor(glm::vec4& color) override;
		virtual void Draw(const Ref<VertexArray>& vertexArray) override;
		virtual void DrawMesh() override;
		virtual void Draw(size_t count) override;
		virtual void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;

	};
}