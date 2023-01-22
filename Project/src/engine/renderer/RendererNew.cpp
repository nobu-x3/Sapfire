#include "RendererNew.h"
#include "engine/renderer/RenderCommands.h"

void Renderer::BeginScene()
{
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const Ref<VertexArray> &vertexArray)
{
	vertexArray->Bind();
	RenderCommands::Draw(vertexArray);
}
