#pragma once

#include "VertexArray.h"
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
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

		Mesh(const std::string& fileName);
		~Mesh();
		virtual void render();

		void set_position(const glm::vec3& pos)
		{
			mWorldPosition = -pos;
			calculate_world_transform();
		}

		void set_rotation(const glm::quat& quat)
		{
			mRotation = quat;
			calculate_world_transform();
		}

		void set_scale(const glm::vec3& scale)
		{
			mScale = scale;
			calculate_world_transform();
		}
		void set_texture(const std::string& path);
		const std::string& get_name() const { return mName; }
		const glm::mat4& get_world_transform() const { return mWorldTransform; }
		const glm::quat& get_rotation() const { return mRotation; }
		void calculate_world_transform();

	private:
		std::string mName;
		std::vector<Vertex> mVertices;
		std::vector<Index> mIndices;
		Ref<VertexBuffer> mVertexBuffer;
		Ref<IndexBuffer> mIndexBuffer;
		Ref<VertexArray> mVertexArray;
		glm::vec3 mWorldPosition;
		glm::quat mRotation;
		glm::vec3 mScale = glm::vec3(1.f);
		glm::mat4 mWorldTransform;
		Ref<class Texture> mTexture;
	};
}