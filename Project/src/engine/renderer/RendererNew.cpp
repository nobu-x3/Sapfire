#include "RendererNew.h"
#include "engine/renderer/Camera.h"
#include "engine/renderer/RenderCommands.h"
#include "engine/renderer/Shader.h"

Matrix4 Renderer::sViewProjection = Matrix4::Identity;

void Renderer::BeginScene(Camera &camera)
{
	sViewProjection = camera.GetViewProjectionMatrix();
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const Ref<VertexArray> &vertexArray, const Ref<Shader> &shader)
{
	shader->Bind();
	shader->SetMatrixUniform("uViewProjection", sViewProjection);
	vertexArray->Bind();
	RenderCommands::Draw(vertexArray);
}
