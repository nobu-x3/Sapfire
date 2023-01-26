#pragma once
#include "engine/renderer/Window.h"
class SDLWindow : public Window
{
	public:
	SDLWindow(const WindowProperties &props);
	virtual ~SDLWindow();
	virtual void OnUpdate() override;
	inline virtual unsigned int GetWidth() const override { return mData.Width; }
	inline virtual unsigned int GetHeight() const override { return mData.Height; }
	virtual void SetEventCallback(const EventCallback &event) override;
	virtual float GetTime() const override;

	private:
	void PollEvents();

	private:
	class SDL_Window *mWindow;
	class RenderingContext *mRenderer;

	struct WindowData
	{
		std::string Title;
		unsigned int Width, Height;
		EventCallback EventCallbackFunction;
	};
	WindowData mData;
};
