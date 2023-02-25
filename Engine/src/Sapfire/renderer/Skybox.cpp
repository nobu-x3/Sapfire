#include "engpch.h"
#include "Skybox.h"

#include "Camera.h"
#include "RenderCommands.h"
#include "Sapfire/renderer/Shader.h"
#include "Sapfire/renderer/Mesh.h"
#include "Sapfire/renderer/CubeMap.h"

namespace Sapfire
{
	Skybox::Skybox(const std::string& shaderPath, std::array<std::string, 6> textureFaces)
	{
		{
			for (int i = 0; i < 36; ++i)
			{
				mIndices[i] = i;
			}
			mVertexArray = VertexArray::create();
			mIndexBuffer = IndexBuffer::create();
			mIndexBuffer->set_data(mIndices, sizeof mIndices);
			BufferLayout layout = {{"inPosition", ShaderDataType::Vec3}};
			mVertexBuffer = VertexBuffer::create();
			mVertexBuffer->set_layout(layout);
			mVertexBuffer->set_data(mVertexData, sizeof mVertexData);
			mVertexArray->add_vertex_buffer(mVertexBuffer);
			mVertexArray->add_index_buffer(mIndexBuffer);
		}
		mCubeMap = CubeMap::create(textureFaces);
		mShader = Shader::create(shaderPath);
	}

	void Skybox::draw()
	{
		RenderCommands::disable_depth();
		RenderCommands::disable_culling();
		mShader->bind();
		mVertexBuffer->bind();
		mCubeMap->bind();
		RenderCommands::draw_skybox();
		RenderCommands::draw(mIndexBuffer->get_size());
		RenderCommands::enable_depth();
		RenderCommands::enable_culling();
	}
}
