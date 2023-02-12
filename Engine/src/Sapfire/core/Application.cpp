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

	Application::Application(const std::string& name) : mRunning(true), mMinimized(false)
	{
		PROFILE_FUNCTION();
		Log::Init();
		sInstance = this;
		mWindow = std::unique_ptr<Window>(Window::Create(WindowProperties(name)));
		mWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		mImguiLayer = new ImguiLayer();
		PushOverlay(mImguiLayer);
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	void Application::Run()
	{
		PROFILE_FUNCTION();
		while (mRunning)
		{
			float timestamp = mWindow->GetTime();
			float deltaTime = timestamp - mLastFrameTime;
			mLastFrameTime = timestamp;
			if (!mMinimized)
			{
				{
					PROFILE_SCOPE("Application: LayerStack OnUpdate");
					for (Layer* layer : mLayerStack)
						layer->OnUpdate(deltaTime);
				}
			}

			{
				PROFILE_SCOPE("Application: ImGui OnRender");
				mImguiLayer->Begin();
				for (Layer* layer : mLayerStack)
				{
					layer->OnImguiRender();
				}
				mImguiLayer->End();
			}
			mWindow->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		mRunning = false;
		return true;
	}

	bool Application::OnWindowResize(class WindowResizeEvent& e)
	{
		PROFILE_FUNCTION();
		if (e.GetWidth() == 0 || e.GetHeight() == 0) { mMinimized = true; return false; }
		mMinimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}