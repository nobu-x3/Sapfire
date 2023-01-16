#pragma once
class Texture
{
	public:
	Texture() = default;
	~Texture() = default;
	bool Load(const std::string &fileName);
	void Unload();
	void SetActive();
	inline int GetWidth() const { return mWidth; }
	inline int GetHeight() const { return mHeight; }

	private:
	// OpenGL id
	unsigned int mTextureID;

	int mWidth;
	int mHeight;
};
