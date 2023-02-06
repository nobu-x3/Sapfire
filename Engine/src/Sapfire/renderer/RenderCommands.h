#pragma once

#include "Sapfire/renderer/RendererAPI.h"
class RenderCommands
{
	public:
	inline static void Init() { sRendererAPI->Init(); }
	inline static void Draw(const Ref<VertexArray> &vertexArray) { sRendererAPI->Draw(vertexArray); }
	inline static void Draw(size_t count) { sRendererAPI->Draw(count); }
	inline static void DrawMesh() { sRendererAPI->DrawMesh(); }
	inline static void SetClearColor(glm::vec4 &color) { sRendererAPI->SetClearColor(color); }
	inline static void ClearScreen() { sRendererAPI->ClearScreen(); }

	private:
	static RendererAPI *sRendererAPI;
};
