#pragma once
class SDL_Window;
class SDL_Renderer;
struct Vector2
{
    float x;
    float y;
};
class Game
{
public:
    Game();

    bool Initialize();
    void Update();
    void Shutdown();

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;

    Vector2 mPaddlePos;
    Vector2 mBallPos;
    int mTicksCount;
    bool mIsRunning;
    int mPaddleDir;
    
};
