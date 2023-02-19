#include "engpch.h"
#include "Skybox.h"

#include "Camera.h"
#include "RenderCommands.h"
#include "Sapfire/renderer/Shader.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/CubeMap.h"

namespace Sapfire
{
	Skybox::Skybox(const std::string& meshPath, const std::string& shaderPath, std::array<std::string, 6> textureFaces)
	{
		{
			mVertexArray = VertexArray::create();
			BufferLayout layout = {{"inPosition", ShaderDataType::Vec3}};
			mVertexBuffer = VertexBuffer::create();
			mVertexBuffer->set_layout(layout);
			mVertexBuffer->set_data(mVertexData, sizeof mVertexData);
			mVertexArray->add_vertex_buffer(mVertexBuffer);
		}
		mMesh = create_ref<Mesh>(meshPath);
		mMesh->set_position({0.f, 0.f, 0.f});
		mCubeMap = CubeMap::create(textureFaces);
		mShader = Shader::create(shaderPath);
	}

	void Skybox::draw(const Camera& camera)
	{
		RenderCommands::disable_depth();
		RenderCommands::disable_culling();
		mShader->bind();
		mShader->set_matrix_uniform("uProjectionMatrix", camera.get_projection_matrix());
		mShader->set_matrix_uniform("uViewMatrix", glm::mat4(glm::mat3(camera.get_view_matrix())));
		// auto& vao = mMesh->get_vertex_array();
		mVertexBuffer->bind();
		mCubeMap->bind();
		RenderCommands::draw_skybox(mVertexBuffer, mCubeMap);
		RenderCommands::draw(108);
		// mMesh->render(false);
		RenderCommands::enable_culling();
		RenderCommands::enable_depth();
	}
}
