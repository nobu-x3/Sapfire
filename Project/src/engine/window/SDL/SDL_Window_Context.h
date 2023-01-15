#pragma once
#include "engine/window/Window.h"

class SDL_Window_Context : public Window {
public:
  SDL_Window_Context(const WindowProperties &props);

  inline virtual unsigned int GetWidth() override { return mWinData.Width; }
  inline virtual unsigned int GetHeight() override { return mWinData.Height; }
  inline virtual void *GetNativeWindow() override { return mWindowHandle; }
  virtual void OnBegin() override;
  virtual void OnEnd() override;
  virtual void OnSubmit() override;

private:
  class SDL_Window *mWindowHandle;
  class RenderingContext *mRenderingContext;

  struct WindowData {
    unsigned int Height, Width;
    std::string Title;
  };

  WindowData mWinData;
};
