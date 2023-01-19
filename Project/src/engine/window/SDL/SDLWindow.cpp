#include "SDLWindow.h"
#include "engine/Log.h"
#include "engine/events/KeyEvent.h"
#include "engine/events/MouseEvent.h"
#include "engine/events/WindowEvent.h"
#include "engine/renderer/opengl/OpenGLContext.h"
#include <SDL2/SDL.h>

Window *Window::Create(const WindowProperties &props)
{
	return new SDLWindow(props);
}

SDLWindow::SDLWindow(const WindowProperties &props)
{
	mData.Title = props.Title;
	mData.Width = props.Width;
	mData.Height = props.Height;
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		ENGINE_ERROR("Unable to initialize SDL: {0}", SDL_GetError());
		return;
	}
	mWindow = SDL_CreateWindow(props.Title.c_str(), 100, 100, static_cast<int>(props.Width),
				   static_cast<int>(props.Height), SDL_WINDOW_OPENGL);

	if (!mWindow)
	{
		ENGINE_ERROR("Unable to initialize window: {0}", SDL_GetError());
		return;
	}

	mRenderer = new OpenGLContext(mWindow);
	mRenderer->Init();
}

void SDLWindow::SetEventCallback(const EventCallback &event)
{
	mData.EventCallbackFunction = event;
}

SDLWindow::~SDLWindow()
{
	mRenderer->Shutdown();
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
void SDLWindow::OnUpdate()
{
	SDL_Event event;
	// returns true if there are events in the q
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT: {
			WindowCloseEvent e;
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_WINDOWEVENT_RESIZED: {
			WindowResizeEvent e(event.window.data1,
					    event.window.data2); // i hope this works, cant resize windows with my setup
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_MOUSEMOTION: {
			MouseMovedEvent e(event.motion.x, event.motion.y);
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_MOUSEBUTTONDOWN: {
			MouseButtonPressedEvent e(event.button.button);
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_MOUSEBUTTONUP: {
			MouseButtonReleasedEvent e(event.button.button);
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_MOUSEWHEEL: {
			MouseScrolledEvent e(event.wheel.x, event.wheel.y);
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_KEYDOWN: {
			KeyPressedEvent e(event.key.keysym.sym, event.key.repeat);
			mData.EventCallbackFunction(e);
			break;
		}
		case SDL_KEYUP: {
			KeyReleasedEvent e(event.key.keysym.sym);
			mData.EventCallbackFunction(e);
			break;
		}
		}
	}

	mRenderer->SwapBuffers();
}
