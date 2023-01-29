#pragma once

#include "RendererAPI.h"
#include "engine/Math.h"
class RenderCommands
{
	public:
	inline static void Init() { sRendererAPI->Init(); }
	inline static void Draw(const Ref<VertexArray> &vertexArray) { sRendererAPI->Draw(vertexArray); }
	inline static void SetClearColor(Vector4 &color) { sRendererAPI->SetClearColor(color); }
	inline static void ClearScreen() { sRendererAPI->ClearScreen(); }

	private:
	static RendererAPI *sRendererAPI;
};
