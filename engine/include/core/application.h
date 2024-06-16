#pragma once

#include <functional>
#include <windows.h>
#include "core/layer_stack.h"
#include "core/logger.h"
#include "core/stl/unique_ptr.h"
#include "events/application_event.h"
#include "events/event.h"
#include "events/input_event.h"
#include "events/keyboard_event.h"

namespace Sapfire {

	constexpr f32 FRAME_STATS_PERIOD = 1.0f;

	struct ApplicationCreationDesc {
		stl::string_view name = "";
		u32 width = 800;
		u32 height = 600;
		LRESULT (*window_proc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = nullptr;
	};

	namespace d3d {
		class Renderer;
		class GraphicsDevice;
	} // namespace d3d

	class Layer;
	class Window;

	struct ClientExtent {
		u64 width = 0;
		u64 height = 0;
	};

	class Application {
	public:
		Application(const ApplicationCreationDesc& desc);
		virtual ~Application();
		void run();
		void close();
		void push_layer(Layer* layer);
		void push_overlay(Layer* layer);
		void on_event(Event& e);
		void submit_to_main_thread(const std::function<void()>& func);
		ClientExtent* client_extent() { return &m_ClientExtent; }
		Window* window() const { return m_Window.get(); }
		static Application& get();

	private:
		void execute_main_thread_queue();

	protected:
		virtual bool on_window_close(WindowCloseEvent& e);
		virtual bool on_window_resize(WindowResizeEvent& e);
		virtual bool on_window_resize_finished(WindowResizeFinishedEvent& e);
		virtual bool on_mouse_button(MouseButtonEvent& e);
		virtual bool on_mouse_moved(MouseMovedEvent& e);
		virtual bool on_key_pressed(KeyPressedEvent& e);
		virtual bool on_key_released(KeyReleasedEvent& e);

	protected:
		virtual void event_loop(f32 delta_time) { CORE_INFO("HELOOO"); };

	protected:
		stl::string m_Name;
		LayerStack m_LayerStack;
		stl::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;
		stl::vector<std::function<void()>> m_MainThreadQueue;
		stl::mutex m_MainThreadQueueMutex;
		ClientExtent m_ClientExtent;
		static Application* s_Instance;
	};

	// Must be defined in client
	Application* create_application();
} // namespace Sapfire
