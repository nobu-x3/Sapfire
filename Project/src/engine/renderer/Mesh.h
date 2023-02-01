#pragma once

#include "engine/renderer/Buffer.h"

#define VERTEX_NUM_ATTRIBUTES 5

class Mesh
{
	public:
	struct Vertex
	{
		Vector3 Position;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 Binormal;
		Vector2 Texcoord;
	};

	struct Index
	{
		uint32_t V1, V2, V3;
	};

	Mesh(const std::string &fileName);
	~Mesh();
	void Render();
	inline const std::string &GetName() const { return mName; }

	private:
	std::string mName;
	std::vector<Vertex> mVertices;
	std::vector<Index> mIndices;
	Ref<VertexBuffer> mVertexBuffer;
	Ref<IndexBuffer> mIndexBuffer;
};
