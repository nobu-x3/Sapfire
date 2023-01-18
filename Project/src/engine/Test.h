#pragma once
#include "engine/Core.h"
#include "engine/LayerStack.h"
class TestApp
{
	public:
	TestApp();
	~TestApp();

	public:
	void Tick();

	private:
	void OnEvent(class Event &e);
	bool OnWindowClose(class WindowCloseEvent &e);

	private:
	Scope<class Window> mWindow;
	LayerStack mLayerStack;

	bool mRunning;
};
