#pragma once
#include "Sapfire/core/Core.h"
#include "Sapfire/core/LayerStack.h"
#include "Sapfire/renderer/Camera.h"
#include "Sapfire/imgui/ImguiLayer.h"

namespace Sapfire
{
	class Application
	{
	public:
		Application();
		~Application();
		void Run();
		inline const class Window& GetWindow() const { return *mWindow; }
		inline void PushLayer(Layer* layer) { mLayerStack.PushLayer(layer); }
		inline void PushOverlay(Layer* layer) { mLayerStack.PushOverlay(layer); }
		inline static Application& GetInstance() { return *sInstance; }
		inline const ImguiLayer& GetImguiLayer() const { return *mImguiLayer; }

	private:
		void OnEvent(class Event& e);
		bool OnWindowClose(class WindowCloseEvent& e);

	private:
		Scope<class Window> mWindow;
		LayerStack mLayerStack;
		float mLastFrameTime = 0.f;
		static Application* sInstance;
		ImguiLayer* mImguiLayer;
		bool mRunning;
		uint32_t mVertexArray;
	};

	// Must be defined in the client.
	// TODO: Add namespaces
	Application* CreateApplication();
}