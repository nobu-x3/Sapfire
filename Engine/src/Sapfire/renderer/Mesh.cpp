#include "engpch.h"
#include "Mesh.h"
#include "Sapfire/renderer/RenderCommands.h"
#include "Sapfire/renderer/Texture.h"
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

namespace Sapfire
{
	const unsigned int IMPORT_FLAGS = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType |
		aiProcess_PreTransformVertices | aiProcess_GenNormals | aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes | aiProcess_Debone | aiProcess_ValidateDataStructure;

	Mesh::Mesh(const std::string& fileName) : mWorldTransform(1.f)
	{
		PROFILE_FUNCTION();
		std::filesystem::path p = fileName;
		mName = p.stem().string();
		ENGINE_INFO("Parsing mesh: {0}", fileName);
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(fileName, IMPORT_FLAGS);
		if (!scene || !scene->HasMeshes())
		{
			ENGINE_ERROR("Failed to parse mesh {0}", fileName);
			return;
		}
		aiMesh* mesh = scene->mMeshes[0];
		// Vertex buffer
		mVertices.reserve(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mVertices[i].y, mesh->mNormals[i].z };
			if (mesh->HasTextureCoords(0))
			{
				vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			mVertices.push_back(vertex);
		}
		mVertexBuffer = VertexBuffer::create();
		BufferLayout layout = { {"inPosition", ShaderDataType::Vec3},
					   {"inNormal", ShaderDataType::Vec3},
					   {"inTangent", ShaderDataType::Vec3},
					   {"inBinormal", ShaderDataType::Vec3},
					   {"inTexCoord", ShaderDataType::Vec3} };
		mVertexBuffer->set_layout(layout);
		mVertexBuffer->set_data(mVertices.data(), mVertices.size() * sizeof(Vertex));
		// Index buffer
		mIndices.reserve(mesh->mNumFaces);
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			if (mesh->mFaces[i].mNumIndices != 3)
			{
				ENGINE_ERROR("Must have 3 indices");
			}
			mIndices.push_back(
				{ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
		}
		mIndexBuffer = IndexBuffer::create();
		mIndexBuffer->set_data(mIndices.data(), mIndices.size() * sizeof(Index));
		mVertexArray = VertexArray::create();
		mVertexArray->add_vertex_buffer(mVertexBuffer);
		mVertexArray->add_index_buffer(mIndexBuffer);
		ENGINE_INFO("Done parsing mesh: {0}", fileName);
	}
	Mesh::~Mesh()
	{
	}

	void Mesh::set_texture(const std::string& path)
	{
		PROFILE_FUNCTION();
		mTexture = Texture::create(path);
	}

	void Mesh::render(bool bindTexture)
	{
		PROFILE_FUNCTION();
		mVertexBuffer->bind();
		mIndexBuffer->bind();
		// mVertexArray->bind();
		if(bindTexture)
			mTexture->bind();
		RenderCommands::draw_mesh();
		RenderCommands::draw(mIndexBuffer->get_size());
	}

	void Mesh::calculate_world_transform()
	{
		PROFILE_FUNCTION();
		mWorldTransform = translate(glm::mat4(1.f), mWorldPosition);
		mWorldTransform *= mat4_cast(mRotation);
		mWorldTransform *= scale(glm::mat4(1.f), mScale);
	}
}