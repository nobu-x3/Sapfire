#include "Mesh.h"
#include "engine/renderer/RenderCommands.h"
#include "engine/renderer/Texture.h"
#include <GL/glew.h> // TODO: make API-agnostic
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

const unsigned int IMPORT_FLAGS = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType |
				  aiProcess_PreTransformVertices | aiProcess_GenNormals | aiProcess_GenUVCoords |
				  aiProcess_OptimizeMeshes | aiProcess_Debone | aiProcess_ValidateDataStructure;

Mesh::Mesh(const std::string &fileName) : mWorldTransform(1.f)
{
	std::filesystem::path p = fileName;
	mName = p.stem().string();
	ENGINE_INFO("Parsing mesh: {0}", fileName);
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fileName, IMPORT_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		ENGINE_ERROR("Failed to parse mesh {0}", fileName);
	}
	aiMesh *mesh = scene->mMeshes[0];
	// Vertex buffer
	mVertices.reserve(mesh->mNumVertices);
	for (int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
		vertex.Normal = {mesh->mNormals[i].x, mesh->mVertices[i].y, mesh->mNormals[i].z};
		if (mesh->HasTextureCoords(0))
		{
			vertex.Texcoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
		}
		mVertices.push_back(vertex);
	}
	mVertexBuffer = VertexBuffer::Create();
	BufferLayout layout = {{"inPosition", ShaderDataType::Vec3},
			       {"inNormal", ShaderDataType::Vec3},
			       {"inTangent", ShaderDataType::Vec3},
			       {"inBinormal", ShaderDataType::Vec3},
			       {"inTexCoord", ShaderDataType::Vec3}};
	mVertexBuffer->SetLayout(layout);
	mVertexBuffer->SetData(mVertices.data(), mVertices.size() * sizeof(Vertex));
	// Index buffer
	mIndices.reserve(mesh->mNumFaces);
	for (int i = 0; i < mesh->mNumFaces; ++i)
	{
		if (mesh->mFaces[i].mNumIndices != 3)
		{
			ENGINE_ERROR("Must have 3 indices");
		}
		mIndices.push_back(
		    {mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]});
	}
	mIndexBuffer = IndexBuffer::Create();
	mIndexBuffer->SetData(mIndices.data(), mIndices.size() * sizeof(Index));
	ENGINE_INFO("Done parsing mesh: {0}", fileName);
}
Mesh::~Mesh()
{
}

void Mesh::SetTexture(const std::string &path)
{
	mTexture = Texture::Create(path);
}

void Mesh::Render()
{
	mVertexBuffer->Bind();
	mIndexBuffer->Bind();
	mTexture->Bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, Tangent));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, Binormal));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, Texcoord));
	RenderCommands::Draw(mIndexBuffer->GetCount());
}

void Mesh::CalculateWorldTransform()
{
	mWorldTransform = glm::translate(glm::mat4(1.f), mWorldPosition);
	mWorldTransform *= glm::mat4_cast(mRotation);
	mWorldTransform *= glm::scale(glm::mat4(1.f), mScale);
}
