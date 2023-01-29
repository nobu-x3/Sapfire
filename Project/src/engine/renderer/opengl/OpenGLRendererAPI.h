#pragma once

#include "engine/renderer/RendererAPI.h"

class OpenGLRendererAPI : public RendererAPI
{
	public:
	virtual void Init() override;
	virtual void ClearScreen() override;
	virtual void SetClearColor(Vector4 &color) override;
	virtual void Draw(const Ref<VertexArray> &vertexArray) override;
};
