#pragma once
#include "Sapfire/core/Core.h"
#include "Sapfire/core/LayerStack.h"
#include "Sapfire/imgui/ImguiLayer.h"

namespace Sapfire
{
	class Application
	{
	public:
		Application(const std::string& name = "");
		~Application();
		void run();
		const class Window& get_window() const { return *mWindow; }
		void push_layer(Layer* layer) { mLayerStack.push_layer(layer); layer->on_attach(); }
		void push_overlay(Layer* layer) { mLayerStack.push_overlay(layer); layer->on_attach(); }
		void pop_layer(Layer* layer) { mLayerStack.pop_layer(layer); layer->on_detach(); }
		void pop_overlay(Layer* layer) { mLayerStack.pop_overlay(layer); layer->on_detach(); }
		static Application& get_instance() { return *sInstance; }
		ImguiLayer& get_imgui_layer() const { return *mImguiLayer; }

	private:
		void on_event(Event& e);
		bool on_window_close(class WindowCloseEvent& e);
		bool on_window_resize(class WindowResizeEvent& e);

	private:
		Scope<Window> mWindow;
		LayerStack mLayerStack;
		float mLastFrameTime = 0.f;
		static Application* sInstance;
		ImguiLayer* mImguiLayer;
		uint32_t mVertexArray;
		bool mRunning;
		bool mMinimized;
	};

	// Must be defined in the client.
	Application* create_application();
}