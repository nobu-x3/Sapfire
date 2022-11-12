#include <SDL.h>
#include "Game.h"

#include <iostream>

const int thickness = 15;
const int paddleH = 150;
Game::Game()
{
    mWindow = nullptr;
    mIsRunning = true;
    mBallPos = { 500, 540 };
    mPaddlePos = {thickness/2.f, 540 };
    mTicksCount = 0;
    mPaddleDir = 0;
    mBallVel = { -200.f, 235.f };
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
        if (state[SDL_SCANCODE_ESCAPE])
            mIsRunning = false;

        mPaddleDir = 0;
        if(state[SDL_SCANCODE_W])
            mPaddleDir -= 1;
        if(state[SDL_SCANCODE_S])
            mPaddleDir += 1;
    }
}

void Game::UpdateGame()
{
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)); // limits to 60 fps

    // calc delta time
    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.f;
    mTicksCount = SDL_GetTicks();

    // clamp delta time to avoid big simulation jumps during debugging
    if(deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    std::cout << mPaddlePos.x << " " << mPaddlePos.y << std::endl; 
    if(mPaddleDir != 0)
    {
        mPaddlePos.y += mPaddleDir * 300.0f * deltaTime;
        if(mPaddlePos.y < paddleH/2.f - thickness/2.f)
            mPaddlePos.y = paddleH/2.f;
        else if(mPaddlePos.y > 1080 - paddleH/2.f - thickness/2.f)
            mPaddlePos.y = 1080 - paddleH/2.f - thickness/2.f;
    }

    mBallPos.x += mBallVel.x * deltaTime;
    mBallPos.y += mBallVel.y * deltaTime;

    if((mBallPos.y <= thickness && mBallVel.y < 0.0f) || (mBallPos.y >= 1080 - thickness && mBallVel.y > 0))
    {
        mBallVel.y *= -1;
    }
    if(/*(mBallPos.x <= thickness && mBallVel.y < 0.0f) || */(mBallPos.x >= 1920 - thickness && mBallVel.x > 0))
        mBallVel.x *= -1;

    auto diff = std::abs(mBallPos.y - mPaddlePos.y);
    if(diff <= paddleH /2.f && mBallPos.x <= mPaddlePos.x + thickness/2.f && mBallPos.x > 0 && mBallVel.x < 0)
    {
        mBallVel.x *= -1;
    }
    else if(mBallPos.x < 0)
        mIsRunning = false;
}

void Game::GenerateOutput()
{
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255); // set color
    SDL_RenderClear(mRenderer); // clear
    SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

    // SDL_Rect leftWall { 0, 0, thickness, 1080};
    // SDL_RenderFillRect(mRenderer, &leftWall);

    SDL_Rect wall { 1920-thickness, 0, thickness, 1080};
    SDL_RenderFillRect(mRenderer, &wall);

    wall = { 0, 0, 1920, thickness };
    SDL_RenderFillRect(mRenderer, &wall);

    wall = { 0, 1080 - thickness, 1920, thickness};
    SDL_RenderFillRect(mRenderer, &wall);
    
    SDL_Rect paddle{static_cast<int>(mPaddlePos.x), static_cast<int>(mPaddlePos.y - paddleH/2.f), thickness, paddleH};
    SDL_RenderFillRect(mRenderer, &paddle);
    SDL_Rect ball
    {
        static_cast<int>(mBallPos.x - thickness / 2), static_cast<int>(mBallPos.y - thickness / 2), thickness, thickness
    };
    SDL_RenderFillRect(mRenderer, &ball);
    SDL_RenderPresent(mRenderer); // swap buffers
}
