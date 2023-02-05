#include "engine/Application.h"
#include "engine/Log.h"
#include "engine/events/WindowEvent.h"
#include "engine/renderer/Buffer.h"
#include "engine/renderer/RenderCommands.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/VertexArray.h"
#include "engine/renderer/Window.h"

Application::Application() : mRunning(true)
{
	mWindow = std::unique_ptr<Window>(Window::Create());
	mWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	Log::Init();
}

Application::~Application()
{
}

void Application::OnEvent(Event &event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

	for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
	{
		(*--it)->OnEvent(event);
		if (event.Handled)
			break;
	}
}

void Application::Run()
{

	while (mRunning)
	{
		float timestamp = mWindow->GetTime();
		float deltaTime = timestamp - mLastFrameTime;
		mLastFrameTime = timestamp;
		for (Layer *layer : mLayerStack)
			layer->OnUpdate(deltaTime);
		mWindow->OnUpdate();
	}
}

bool Application::OnWindowClose(WindowCloseEvent &e)
{
	mRunning = false;
	return true;
}
