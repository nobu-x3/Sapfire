#pragma once

#include "engine/renderer/Buffer.h"

class Mesh
{
	public:
	Mesh() = default;
	~Mesh() = default;
	bool Load(const std::string &fileName, class Renderer *renderer);
	void Unload();
	inline class VertexBuffer *GetVertexBuffer() const { return mVertexBuffer; }
	inline class IndexBuffer *GetIndexBuffer() const { return mIndexBuffer; }
	class Texture *GetTexture(int index) const;
	inline const std::string &GetShaderName() const { return mShaderName; }
	inline float GetRadius() const { return mRadius; }
	inline float GetSpecPower() const { return mSpecPower; }

	private:
	std::vector<class Texture *> mTextures;
	class OpenGLShader *mShader;
	class VertexBuffer *mVertexBuffer;
	class IndexBuffer *mIndexBuffer;
	std::string mShaderName;
	float mSpecPower;
	// object space bounding sphere radius
	float mRadius;
};
