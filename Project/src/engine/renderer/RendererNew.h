#pragma once

#include "RendererAPI.h"

enum class WindowAPI
{
	None = 0,
	SDL = 1,
};

class Renderer
{
	public:
	static void BeginScene();
	static void EndScene();
	static void Submit(const Ref<VertexArray> &vertexArray);
};
