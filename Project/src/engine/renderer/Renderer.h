#pragma once

#include "engine/Math.h"
#include <SDL2/SDL.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

struct DirectionalLight
{
	Vector3 mDirection;
	Vector3 mDiffuseColor;
	Vector3 mSpecColor;
};

struct PointLight
{
	Vector3 mPosition;
	Vector3 mDiffuseColor;
	Vector3 mSpecColor;
	float mIntensity;
	float mRadius;
};

class Renderer
{
	public:
	Renderer() = default;
	Renderer(class Game *game);
	~Renderer() = default;
	bool Initialize(float width, float height);
	void Shutdown();
	void UnloadData();
	void Draw();
	void AddSprite(class SpriteComponent *sprite);
	void RemoveSprite(class SpriteComponent *sprite);
	void AddMeshComponent(class MeshComponent *mesh);
	void RemoveMeshComponent(class MeshComponent *mesh);
	inline void SetViewMatrix(const Matrix4 &view) { mView = view; }
	class Texture *GetTexture(const char *fileName);
	class Mesh *GetMesh(const char *fileName);
	class Shader *GetShader(const std::string &fileName);
	void LoadShader(class Shader *sh);
	void LinkShaderToMeshComp(const std::string &fileName, class MeshComponent *meshComp);
	inline float GetScreenWidth() const { return mScreenWidth; }
	inline float GetScreenHeight() const { return mScreenHeight; }
	inline void SetAmbientLight(const Vector3 &light) { mAmbientLight = light; }
	inline DirectionalLight &GetDirectionalLight() { return mDirectionalLight; }
	inline std::array<PointLight, 4> &GetPointLights() { return mPointLights; }

	private:
	void CreateSpriteVerts();
	bool LoadShaders();
	void SetLightUniforms(class Shader *shader);

	std::unordered_map<std::string, class Texture *> mTextures;
	std::unordered_map<std::string, class Mesh *> mMeshes;
	std::unordered_map<std::string, class Shader *> mShaders;
	std::unordered_map<class Shader *, std::vector<class MeshComponent *>> mShaderMeshCompMap;
	std::vector<class SpriteComponent *> mSprites; // this list is sorted
	std::vector<class MeshComponent *> mMeshComponents;
	std::vector<class Shader *> mMeshShaders;

	class VertexArray *mSpriteVerts;
	class Shader *mSpriteShader;

	class SDL_Window *mWindow;
	SDL_GLContext mContext;
	class Game *mGame;

	// View/projection for 3D shaders
	Matrix4 mView;
	Matrix4 mProjection;
	// Width/height of screen
	float mScreenWidth;
	float mScreenHeight;

	Vector3 mAmbientLight;
	DirectionalLight mDirectionalLight;
	std::array<PointLight, 4> mPointLights;
};
