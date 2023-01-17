#include "engine/Test.h"
#include "engine/window/Window.h"
TestApp::TestApp()
{
	mWindow = std::unique_ptr<Window>(Window::Create());
}

TestApp::~TestApp()
{
}
