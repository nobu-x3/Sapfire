#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <vector>
class Renderer
{
	public:
	Renderer() = default;
	~Renderer() = default;
	bool Initialize(float width, float height);
	void Shutdown();
	void UnloadData();
	void Draw();
	void AddSprite(class SpriteComponent *sprite);
	void RemoveSprite(class SpriteComponent *sprite);
	class Texture *LoadTexture(const char *fileName);
	inline float GetScreenWidth() const { return mScreenWidth; }
	inline float GetScreenHeight() const { return mScreenHeight; }

	private:
	void CreateSpriteVerts();
	bool LoadShaders();

	std::unordered_map<std::string, class Texture *> mTextures;
	std::vector<SpriteComponent *> mSprites; // this list is sorted

	class VertexArray *mSpriteVerts;
	class Shader *mSpriteShader;

	class SDL_Window *mWindow;
	SDL_GLContext mContext;
	class Game *mGame;

	float mScreenWidth;
	float mScreenHeight;
};
