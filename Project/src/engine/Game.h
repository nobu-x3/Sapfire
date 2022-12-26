﻿#pragma once
#include "Actor.h"
#include <SDL_render.h>
#include <memory>
#include <string>
#include <unordered_map>
class SDL_Window;
class SDL_Renderer;
class SpriteComponent;
class Game
{
public:
    Game();
    ~Game();

    bool Initialize();
    void Update();
    void Shutdown();

	void AddActor(Actor* actor);
	void RemoveActor(Actor *actor);
	void AddSprite(SpriteComponent *sprite);
	void RemoveSprite(SpriteComponent *sprite);
	class Texture *LoadTexture(const char *fileName);
	void AddAsteroid(class Asteroid *ast);
	void RemoveAsteroid(class Asteroid *ast);
	void NotifyShipDeath();
	inline std::vector<class Asteroid *> GetAsteroids() const { return mAsteroids; }

	private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();
	void RespawnShip();
	void CreateSpriteVerts();
	bool LoadShaders();

	SDL_Window *mWindow;
	/* SDL_Renderer *mRenderer; */
	SDL_GLContext mContext;

	int mTicksCount;
	bool mIsRunning;
	// Map of textures loaded
	std::unordered_map<std::string, class Texture *> mTextures;
	std::vector<Actor*> mActors;
	std::vector<SpriteComponent *> mSprites; // this list is sorted
	std::vector<Actor*> mPendingActors;

	/* std::unique_ptr<class VertexArray> mSpriteVerts; */
	/* std::unique_ptr<class Shader> mSpriteShader; */
	class VertexArray *mSpriteVerts;
	class Shader *mSpriteShader;

	float mShipRespawnCooldown;
	bool mShipDead;

	class Ship *mShip;
	std::vector<class Asteroid *> mAsteroids;
	bool mUpdatingActors;
};
