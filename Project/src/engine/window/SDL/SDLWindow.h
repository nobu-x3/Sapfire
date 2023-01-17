#pragma once
#include "engine/window/Window.h"
class SDLWindow : public Window
{
	public:
	SDLWindow(const WindowProperties &props);
	virtual ~SDLWindow();
	virtual void OnUpdate() override;
	inline virtual unsigned int GetWidth() const override { return mProperties.Width; }
	inline virtual unsigned int GetHeight() const override { return mProperties.Height; }

	private:
	class SDL_Window *mWindow;

	WindowProperties mProperties;
};
