#include "engine/Test.h"
#include "engine/events/WindowEvent.h"
#include "engine/window/Window.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
TestApp::TestApp()
{
	mWindow = std::unique_ptr<Window>(Window::Create());
	mWindow->SetEventCallback(BIND_EVENT_FN(TestApp::OnEvent));
}

TestApp::~TestApp()
{
}

void TestApp::OnEvent(Event &event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(TestApp::OnWindowClose));

	std::cout << event.GetName() << ": " << event.ToString() << std::endl;
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
