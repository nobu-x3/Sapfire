#include "engine/Test.h"
#include "engine/Log.h"
#include "engine/events/WindowEvent.h"
#include "engine/window/SDL/SDLWindow.h"
#include "engine/window/Window.h"
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
TestApp::TestApp()
{
	mWindow = std::unique_ptr<Window>(Window::Create());
	mWindow->SetEventCallback(BIND_EVENT_FN(TestApp::OnEvent));
	mLayerStack.PushLayer(new Layer());
	Log::Init();
}

TestApp::~TestApp()
{
}

void TestApp::OnEvent(Event &event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(TestApp::OnWindowClose));

	for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
	{
		(*--it)->OnEvent(event);
		if (event.Handled)
			break;
	}
}

void TestApp::Tick()
{
	while (mRunning)
	{
		mWindow->OnUpdate();
	}
}

bool TestApp::OnWindowClose(WindowCloseEvent &e)
{
	mRunning = false;
	return true;
}
