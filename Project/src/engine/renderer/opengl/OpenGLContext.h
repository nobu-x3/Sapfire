#pragma once

#include "engine/renderer/RenderingContext.h"
#include <GL/glew.h>
class OpenGLContext : public RenderingContext {
public:
  OpenGLContext(class SDL_Window *window);
  virtual ~OpenGLContext() {}
  virtual void Init() override;
  virtual void OnBegin() override;
  virtual void OnEnd() override;
  virtual void SwapWindow() override;

private:
  class SDL_Window *mWindowHandle;
};
