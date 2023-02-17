#include "engpch.h"
#include "Renderer.h"
#include "Sapfire/renderer/Camera.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/RenderCommands.h"
#include "Sapfire/renderer/Shader.h"

namespace Sapfire
{
	Renderer::SceneData* Renderer::sSceneData = new SceneData;

	WindowAPI Renderer::sWindowAPI = WindowAPI::GLFW;

	void Renderer::begin_scene(Camera& camera, const Ref<UniformBuffer>& uniformBuffer)
	{
		PROFILE_FUNCTION();
		sSceneData->ViewProjectionMatrix = camera.get_view_projection_matrix();
		uniformBuffer->set_data(sSceneData);
	}

	void Renderer::end_scene()
	{
		PROFILE_FUNCTION();
	}
	
	void Renderer::submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader)
	{
		PROFILE_FUNCTION();
		shader->bind();
		vertexArray->bind();
		RenderCommands::draw(vertexArray);
	}

	void Renderer::submit_mesh(const Ref<Mesh>& mesh, const Ref<Shader>& shader)
	{
		PROFILE_FUNCTION();
		shader->bind();
		shader->set_matrix_uniform("uWorldTransform", mesh->get_world_transform());
		mesh->render();
	}

	

	void Renderer::on_window_resize(uint16_t width, uint16_t height)
	{
		PROFILE_FUNCTION();
		RenderCommands::set_viewport(0, 0, width, height);
	}
}