#pragma once

#include "Math.h"
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
	void AddMeshComponent(class MeshComponent *mesh);
	void RemoveMeshComponent(class MeshComponent *mesh);
	class Texture *LoadTexture(const char *fileName);
	class Mesh *LoadMesh(const char *fileName);
	inline float GetScreenWidth() const { return mScreenWidth; }
	inline float GetScreenHeight() const { return mScreenHeight; }

	private:
	void CreateSpriteVerts();
	bool LoadShaders();

	std::unordered_map<std::string, class Texture *> mTextures;
	std::unordered_map<std::string, class Mesh *> mMeshes;
	std::vector<class SpriteComponent *> mSprites; // this list is sorted
	std::vector<class MeshComponent *> mMeshComponents;

	class VertexArray *mSpriteVerts;
	class Shader *mSpriteShader;
	class Shader *mMeshShader;

	class SDL_Window *mWindow;
	SDL_GLContext mContext;
	class Game *mGame;

	// View/projection for 3D shaders
	Matrix4 mView;
	Matrix4 mProjection;
	// Width/height of screen
	float mScreenWidth;
	float mScreenHeight;
};
