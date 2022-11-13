#include <SDL.h>
#include "Game.h"

#include <iostream>

const int thickness = 15;
const int paddleH = 150;

Game::Game()
{
	mWindow = nullptr;
	mIsRunning = true;
	mTicksCount = 0;
}

Game::~Game()
{
	while (!mActors.empty())
	{
		delete mActors.back();
	}
}


bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow("Chapter 1", 100, 100, 1920, 1080, 0);
	if (!mWindow)
	{
		SDL_Log("Unable to initialize window: %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!mRenderer)
	{
		SDL_Log("Unable to initialize renderer: %s", SDL_GetError());
		return false;
	}
	return true;
}

void Game::Update()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}
void Game::AddActor(Actor* actor)
{
	if (mUpdatingActors)
		mPendingActors.emplace_back(actor);
	else
		mActors.emplace_back(actor);
}

void Game::ProcessInput()
{
	SDL_Event event;

	// returns true if there are events in the q
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}
}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)); // limits to 60 fps

	// calc delta time
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.f;
	mTicksCount = SDL_GetTicks();

	// clamp delta time to avoid big simulation jumps during debugging
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mUpdatingActors = true;
	for(auto actor : mActors)
	{
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;

	for(auto pending : mPendingActors)
	{
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();

	std::vector<Actor*> actorsPendingRemoval;
	for(auto actor : mActors)
	{
		if (actor->GetState() == Actor::EPendingRemoval)
			actorsPendingRemoval.emplace_back(actor);
	}

	for(auto actor : actorsPendingRemoval)
	{
		delete actor;
	}
}

void Game::GenerateOutput()
{
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255); // set color
	SDL_RenderClear(mRenderer); // clear
	SDL_RenderPresent(mRenderer); // swap buffers
}
