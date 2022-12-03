﻿#pragma once
#include "Actor.h"
#include <SDL_render.h>
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
	void AddSprite(SpriteComponent *sprite);
	void RemoveSprite(SpriteComponent *sprite);
	SDL_Texture *LoadTexture(const char *fileName);

      private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();

	SDL_Window *mWindow;
	SDL_Renderer *mRenderer;

	int mTicksCount;
	bool mIsRunning;
	// Map of textures loaded
	std::unordered_map<std::string, SDL_Texture *> mTextures;
	std::vector<Actor*> mActors;
	std::vector<SpriteComponent *> mSprites; // this list is sorted
	std::vector<Actor*> mPendingActors;

	bool mUpdatingActors;

	class Ship *mShip;
};