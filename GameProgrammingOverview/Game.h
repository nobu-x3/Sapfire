#pragma once
class SDL_Window;
class SDL_Renderer;
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
    bool mIsRunning;
    
};
