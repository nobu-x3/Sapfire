#pragma once
#include "Actor.h"
#include <SDL_render.h>
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

	void AddActor(Actor *actor);
	void RemoveActor(Actor *actor);
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
	std::vector<Actor *> mActors;
	std::vector<Actor *> mPendingActors;

	bool mUpdatingActors;

	class Renderer *mRenderer;
	// Game specific code
	class CameraActor *mCameraActor;
};
