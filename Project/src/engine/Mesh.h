#pragma once
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
	inline float GetSpecPower() const { return mSpecPower; }

	private:
	std::vector<class Texture *> mTextures;
	class Shader *mShader;
	class VertexArray *mVertexArray;
	std::string mShaderName;
	float mSpecPower;
	// object space bounding sphere radius
	float mRadius;
};
