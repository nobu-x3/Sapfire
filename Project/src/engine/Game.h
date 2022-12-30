#pragma once
#include "Actor.h"
#include "game/Asteroid.h"
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
	void AddAsteroid(class Asteroid *ast);
	void RemoveAsteroid(class Asteroid *ast);
	void NotifyShipDeath();
	std::vector<class Asteroid *> GetAsteroids() const;
	inline class Renderer *GetRenderer() { return mRenderer; }

	private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();
	void RespawnShip();


	int mTicksCount;
	bool mIsRunning;
	// Map of textures loaded
	std::vector<Actor*> mActors;
	std::vector<Actor*> mPendingActors;


	bool mUpdatingActors;

	// Game specific code
	float mShipRespawnCooldown;
	bool mShipDead;
	float timer;

	class Ship *mShip;
	class Renderer *mRenderer;
	std::vector<class Asteroid *> mAsteroids;
	class CameraActor *mCameraActor;
};
