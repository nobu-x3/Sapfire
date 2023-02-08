#pragma once

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
		virtual void Render();
		inline void SetPosition(const glm::vec3& pos)
		{
			mWorldPosition = -pos;
			CalculateWorldTransform();
		}
		inline void SetRotation(const glm::quat& quat)
		{
			mRotation = quat;
			CalculateWorldTransform();
		}
		inline void SetScale(const glm::vec3& scale)
		{
			mScale = scale;
			CalculateWorldTransform();
		}
		void SetTexture(const std::string& path);
		inline const std::string& GetName() const { return mName; }
		inline const glm::mat4& GetWorldTransform() const { return mWorldTransform; }
		inline const glm::quat& GetRotation() const { return mRotation; }
		void CalculateWorldTransform();

	private:
		std::string mName;
		std::vector<Vertex> mVertices;
		std::vector<Index> mIndices;
		Ref<VertexBuffer> mVertexBuffer;
		Ref<IndexBuffer> mIndexBuffer;
		glm::vec3 mWorldPosition;
		glm::quat mRotation;
		glm::vec3 mScale = glm::vec3(1.f);
		glm::mat4 mWorldTransform;
		Ref<class Texture> mTexture;
	};
}