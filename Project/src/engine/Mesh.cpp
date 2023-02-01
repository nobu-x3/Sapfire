#include "Mesh.h"
#include "Math.h"
#include "engine/renderer/Buffer.h"
#include "engine/renderer/Renderer.h"
#include "nlohmann/json.hpp"
#include <SDL2/SDL_log.h>
#include <fstream>

#define CURRENT_MESH_VERSION 1
#define VERT_SIZE 8

bool Mesh::Load(const std::string &fileName, Renderer *renderer)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		SDL_Log("File not found: Mesh %s", fileName.c_str());
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(file);
	int ver = data["version"].get<int>();
	if (ver != CURRENT_MESH_VERSION)
	{
		SDL_Log("Mesh %s is not version %d", fileName.c_str(), CURRENT_MESH_VERSION);
		return false;
	}

	mShaderName = data["shader"].get<std::string>();
	renderer->GetShader(mShaderName);
	mSpecPower = data["specularPower"].get<float>();
	// Load textures
	auto textures = data["textures"];
	if (!textures.is_array() || textures.size() < 1)
	{
		SDL_Log("Mesh %s does not contain textures.", fileName.c_str());
		return false;
	}
	for (int i = 0; i < textures.size(); ++i)
	{
		std::string texName = textures[i].get<std::string>();
		Ref<Texture> t = renderer->GetTexture(texName.c_str());
		if (t == nullptr)
		{
			t = renderer->GetTexture(texName.c_str());
			if (t == nullptr)
			{
				t = renderer->GetTexture("../Assets/Default.png");
				if (t == nullptr)
				{
					SDL_Log("No default mesh texture.");
					return false;
				}
			}
		}
		mTextures.emplace_back(t);
	}

	// Load vertices
	auto vertsJson = data["vertices"];
	if (!vertsJson.is_array() || vertsJson.size() < 1)
	{
		SDL_Log("Mesh %s does not contain vertices", fileName.c_str());
		return false;
	}
	std::vector<float> vertices;
	size_t vertSize = VERT_SIZE;
	vertices.reserve(vertsJson.size() * vertSize);
	mRadius = 0.f;
	for (int i = 0; i < vertsJson.size(); ++i)
	{
		auto vert = vertsJson[i];
		if (!vert.is_array() || vert.size() != vertSize)
		{
			SDL_Log("Unexpected vertex format for %s.", fileName.c_str());
			return false;
		}
		Vector3 pos(vert[0].get<float>(), vert[1].get<float>(), vert[2].get<float>());
		mRadius = Math::Max(mRadius, pos.LengthSq());
		for (int j = 0; j < vert.size(); ++j)
		{
			vertices.emplace_back(vert[j].get<float>());
		}
	}

	// Was computing length squared earlier
	mRadius = Math::Sqrt(mRadius);
	// Load indices
	auto indexJson = data["indices"];
	if (!indexJson.is_array() || indexJson.size() < 1)
	{
		SDL_Log("Mesh %s does not contain vertex indices.", fileName.c_str());
		return false;
	}

	std::vector<unsigned int> indices;
	indices.reserve(indexJson.size() * 3);
	for (int i = 0; i < indexJson.size(); ++i)
	{
		auto indArr = indexJson[i];
		if (!indArr.is_array() || indArr.size() != 3)
		{
			SDL_Log("Invalid indices in mesh %s.", fileName.c_str());
			return false;
		}
		indices.emplace_back(indArr[0].get<unsigned int>());
		indices.emplace_back(indArr[1].get<unsigned int>());
		indices.emplace_back(indArr[2].get<unsigned int>());
	}
	/* mVertexBuffer = VertexBuffer::Create(vertices.data(), static_cast<uint32_t>(vertices.size()) / vertSize); */
	BufferLayout layout = {{"inPosition", ShaderDataType::Vec3},
			       {"inNormal", ShaderDataType::Vec3},
			       {"inTexCoord", ShaderDataType::Vec2}};
	mVertexBuffer->SetLayout(layout);
	/* mIndexBuffer = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size())); */
	return true;
}

void Mesh::Unload()
{
	delete mVertexBuffer;
	delete mIndexBuffer;
	mVertexBuffer = nullptr;
	mIndexBuffer = nullptr;
}

Ref<Texture> Mesh::GetTexture(int index) const
{
	if (index < mTextures.size())
	{
		return mTextures[index];
	}
	else
	{
		return nullptr;
	}
}
