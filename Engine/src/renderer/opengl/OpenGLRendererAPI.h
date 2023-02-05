#pragma once

#include "renderer/RendererAPI.h"

class OpenGLRendererAPI : public RendererAPI
{
	public:
	virtual void Init() override;
	virtual void ClearScreen() override;
	virtual void SetClearColor(glm::vec4 &color) override;
	virtual void Draw(const Ref<VertexArray> &vertexArray) override;
	virtual void DrawMesh() override;
	virtual void Draw(size_t count) override;
};
