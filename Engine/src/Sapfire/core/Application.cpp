#include "engpch.h"
#include "Application.h"
#include "Log.h"
#include "Sapfire/events/WindowEvent.h"
#include "Sapfire/renderer/Renderer.h"
#include "Sapfire/renderer/Window.h"
#include "Input.h"

namespace Sapfire
{
	Application* Application::sInstance = nullptr;

	Application::Application(const std::string& name) : mRunning(true), mMinimized(false)
	{
		PROFILE_FUNCTION();
		Log::init();
		sInstance = this;
		mWindow = std::unique_ptr<Window>(Window::create(WindowProperties(name)));
		mWindow->set_event_callback(BIND_EVENT_FN(Application::on_event));
		mImguiLayer = new ImguiLayer();
		push_overlay(mImguiLayer);
	}

	Application::~Application()
	{
	}

	void Application::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::on_window_close));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::on_window_resize));

		for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
		{
			(*--it)->on_event(event);
			if (event.Handled)
				break;
		}
	}

	void Application::run()
	{
		PROFILE_FUNCTION();
		while (mRunning)
		{
			float timestamp = mWindow->get_time();
			float deltaTime = timestamp - mLastFrameTime;
			mLastFrameTime = timestamp;
			if (!mMinimized)
			{
				{
					PROFILE_SCOPE("Application: LayerStack OnUpdate");
					for (Layer* layer : mLayerStack)
						layer->on_update(deltaTime);
				}
			}

			{
				PROFILE_SCOPE("Application: ImGui OnRender");
				mImguiLayer->begin();
				for (Layer* layer : mLayerStack)
				{
					layer->on_imgui_render();
				}
				mImguiLayer->end();
			}
			mWindow->on_update();
		}
	}

	bool Application::on_window_close(WindowCloseEvent& e)
	{
		mRunning = false;
		return true;
	}

	bool Application::on_window_resize(WindowResizeEvent& e)
	{
		PROFILE_FUNCTION();
		if (e.get_width() == 0 || e.get_height() == 0) { mMinimized = true; return false; }
		mMinimized = false;
		Renderer::on_window_resize(e.get_width(), e.get_height());

		return false;
	}

}