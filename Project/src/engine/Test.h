#pragma once
#include "engine/Core.h"
#include "engine/LayerStack.h"
#include "engine/renderer/Camera.h"
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
	Ref<class VertexArray> mVA;
	Ref<class Shader> mShader;
	OrthographicCamera mCamera;
	LayerStack mLayerStack;

	bool mRunning;
	uint32_t mVertexArray;
};
