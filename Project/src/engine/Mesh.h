#pragma once

#include <string>
#include <vector>
class Mesh
{
	public:
	Mesh() = default;
	~Mesh() = default;
	bool Load(const std::string &fileName, class Renderer *renderer);
	void Unload();
	class VertexArray *GetVertexArray() const { return mVertexArray; }
	class Texture *GetTexture(int index) const;
	inline const std::string &GetShaderName() const { return mShaderName; }
	inline float GetRadius() const { return mRadius; }

	private:
	std::vector<class Texture *> mTextures;
	class VertexArray *mVertexArray;
	std::string mShaderName;
	// object space bounding sphere radius
	float mRadius;
};