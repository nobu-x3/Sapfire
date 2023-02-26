#pragma once

#include "VertexArray.h"
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
	class Shader;
#define VERTEX_NUM_ATTRIBUTES 5

	class Mesh
	{
	public:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Binormal;
			glm::vec2 Texcoord;
		};

		struct Index
		{
			uint32_t V1, V2, V3;
		};

		Mesh(const std::string& fileName, const Ref<Shader>& shader);
		~Mesh();
		virtual void render(bool bindTexture = true);
		void set_texture(const std::string& path);
		const std::string& get_name() const { return mName; }
		const Ref<VertexArray>& get_vertex_array() const { return mVertexArray; }
		const Ref<Shader>& get_shader() const { return mShader; }

	private:
		std::string mName;
		std::vector<Vertex> mVertices;
		std::vector<Index> mIndices;
		Ref<VertexBuffer> mVertexBuffer;
		Ref<IndexBuffer> mIndexBuffer;
		Ref<VertexArray> mVertexArray;
		Ref<Shader> mShader;
		Ref<class Texture> mTexture;
	};
}
