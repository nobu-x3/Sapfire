#include "SDL_Window_Context.h"
#include "SDL2/SDL.h"
#include "engine/renderer/opengl/OpenGLContext.h"
Window *Window::Create(const WindowProperties &props) {
  return new SDL_Window_Context(props);
}

SDL_Window_Context::SDL_Window_Context(const WindowProperties &props) {

  mWinData.Title = props.Title;
  mWinData.Width = props.Width;
  mWinData.Height = props.Height;

  int sdlResult = SDL_Init(SDL_INIT_VIDEO);
  if (sdlResult != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return;
  }
  mWindowHandle = SDL_CreateWindow(
      props.Title.c_str(), 100, 100, static_cast<int>(props.Width),
      static_cast<int>(props.Height), SDL_WINDOW_OPENGL);
  if (!mWindowHandle) {
    SDL_Log("Unable to initialize window: %s", SDL_GetError());
    return;
  }
  mRenderingContext = new OpenGLContext(mWindowHandle);
  mRenderingContext->Init();
}

void SDL_Window_Context::OnBegin() { mRenderingContext->OnBegin(); }
void SDL_Window_Context::OnEnd() { mRenderingContext->OnEnd(); }
void SDL_Window_Context::OnSubmit() { mRenderingContext->SwapWindow(); }
