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
		Application(const std::string& name = "");
		~Application();
		void Run();
		inline const class Window& GetWindow() const { return *mWindow; }
		inline void PushLayer(Layer* layer) { mLayerStack.PushLayer(layer); layer->OnAttach(); }
		inline void PushOverlay(Layer* layer) { mLayerStack.PushOverlay(layer); layer->OnAttach(); }
		inline void PopLayer(Layer* layer) { mLayerStack.PopLayer(layer); layer->OnDetach(); }
		inline void PopOverlay(Layer* layer) { mLayerStack.PopOverlay(layer); layer->OnDetach(); }
		inline static Application& GetInstance() { return *sInstance; }
		inline const ImguiLayer& GetImguiLayer() const { return *mImguiLayer; }
		inline ImguiLayer* GetImguiLayer() { return mImguiLayer; }

	private:
		void OnEvent(class Event& e);
		bool OnWindowClose(class WindowCloseEvent& e);
		bool OnWindowResize(class WindowResizeEvent& e);

	private:
		Scope<class Window> mWindow;
		LayerStack mLayerStack;
		float mLastFrameTime = 0.f;
		static Application* sInstance;
		ImguiLayer* mImguiLayer;
		uint32_t mVertexArray;
		bool mRunning;
		bool mMinimized;
	};

	// Must be defined in the client.
	// TODO: Add namespaces
	Application* CreateApplication();
}