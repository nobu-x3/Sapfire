#include "SDLWindow.h"
#include "SDL2/SDL.h"

Window *Window::Create(const WindowProperties &props)
{
	return new SDLWindow(props);
}

SDLWindow::SDLWindow(const WindowProperties &props)
{
	mProperties = props;
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return;
	}
	mWindow = SDL_CreateWindow(props.Title.c_str(), 100, 100, static_cast<int>(props.Width),
				   static_cast<int>(props.Height), SDL_WINDOW_OPENGL);
	if (!mWindow)
	{
		SDL_Log("Unable to initialize window: %s", SDL_GetError());
		return;
	}
}

SDLWindow::~SDLWindow()
{
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
void SDLWindow::OnUpdate()
{
	SDL_GL_SwapWindow(mWindow);
}
