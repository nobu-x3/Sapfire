#include "engpch.h"
#include "Application.h"
#include "Log.h"
#include "Sapfire/events/WindowEvent.h"
#include "Sapfire/renderer/Buffer.h"
#include "Sapfire/renderer/RenderCommands.h"
#include "Sapfire/renderer/Renderer.h"
#include "Sapfire/renderer/Shader.h"
#include "Sapfire/renderer/VertexArray.h"
#include "Sapfire/renderer/Window.h"
#include "Input.h"

namespace Sapfire
{
	Application* Application::sInstance = nullptr;

	Application::Application() : mRunning(true)
	{
		Log::Init();
		sInstance = this;
		mWindow = std::unique_ptr<Window>(Window::Create());
		mWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		mImguiLayer = new ImguiLayer();
		PushOverlay(mImguiLayer);
	}

	Application::~Application()
	{
		Input::Deinit();
	}

	void Application::OnEvent(Event& event)
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
			for (Layer* layer : mLayerStack)
				layer->OnUpdate(deltaTime);

			mImguiLayer->Begin();
			for (Layer* layer : mLayerStack)
			{
				layer->OnImguiRender();
			}
			mImguiLayer->End();
			mWindow->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		mRunning = false;
		return true;
	}
}