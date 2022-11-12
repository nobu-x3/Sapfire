#include <SDL.h>
#include "Game.h"

Game::Game()
{
    mWindow = nullptr;
    mIsRunning = true;
}

bool Game::Initialize()
{
    int sdlResult = SDL_Init(SDL_INIT_VIDEO);
    if (sdlResult != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Chapter 1", 100, 100, 800, 600, 0);
    if (!mWindow)
    {
        SDL_Log("Unable to initialize window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!mRenderer)
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

        const Uint8* state = SDL_GetKeyboardState(nullptr);
        if(state[SDL_SCANCODE_ESCAPE])
            mIsRunning = false;
        
    }
}

void Game::UpdateGame()
{
}

void Game::GenerateOutput()
{
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mRenderer);
    SDL_RenderPresent(mRenderer);
}
