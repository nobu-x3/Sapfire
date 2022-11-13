#pragma once
#include "Structs.h"
#include "Actor.h"
class SDL_Window;
class SDL_Renderer;
class Game
{
public:
    Game();
    ~Game();

    bool Initialize();
    void Update();
    void Shutdown();

	void AddActor(Actor* actor);

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;

    int mTicksCount;
    bool mIsRunning;

	std::vector<Actor*> mActors;
	std::vector<Actor*> mPendingActors;

	bool mUpdatingActors;

};
