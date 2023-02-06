#include "engpch.h"
#include "Renderer.h"
#include "Sapfire/renderer/Camera.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/RenderCommands.h"
#include "Sapfire/renderer/Shader.h"

Renderer::SceneData *Renderer::sSceneData = new Renderer::SceneData;

WindowAPI Renderer::sWindowAPI = WindowAPI::GLFW;

void Renderer::BeginScene(Camera &camera)
{
	sSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const Ref<VertexArray> &vertexArray, const Ref<Shader> &shader)
{
	shader->Bind();
	shader->SetMatrixUniform("uViewProjection", sSceneData->ViewProjectionMatrix);
	vertexArray->Bind();
	RenderCommands::Draw(vertexArray);
}

void Renderer::SubmitMesh(const Ref<Mesh> &mesh, const Ref<Shader> &shader)
{
	shader->Bind();
	shader->SetMatrixUniform("uViewProj", sSceneData->ViewProjectionMatrix);
	shader->SetMatrixUniform("uWorldTransform", mesh->GetWorldTransform());
	mesh->Render();
}
