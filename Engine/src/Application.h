#pragma once
#include "engine/Core.h"
#include "engine/LayerStack.h"
#include "engine/renderer/Camera.h"

class Application
{
	public:
	Application();
	~Application();
	void Run();
	inline class Window &GetWindow() { return *mWindow; }
	inline void PushLayer(Layer *layer) { mLayerStack.PushLayer(layer); }
	inline void PushOverlay(Layer *layer) { mLayerStack.PushOverlay(layer); }

	private:
	void OnEvent(class Event &e);
	bool OnWindowClose(class WindowCloseEvent &e);

	private:
	Scope<class Window> mWindow;
	LayerStack mLayerStack;
	float mLastFrameTime = 0.f;

	bool mRunning;
	uint32_t mVertexArray;
};

// Must be defined in the client.
// TODO: Add namespaces
Application *CreateApplication();
