#pragma once

#include "renderer/RenderingContext.h"
#include <GL/glew.h>
#include <SDL.h>

class OpenGLContext : public RenderingContext
{
	public:
	OpenGLContext(class SDL_Window *window);
	virtual ~OpenGLContext() {}
	virtual void Init() override;
	virtual void SwapBuffers() override;
	virtual void Shutdown() override;

	private:
	class SDL_Window *mWindowHandle;
	SDL_GLContext mGlContext;
};
