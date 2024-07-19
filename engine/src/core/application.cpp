#include "engpch.h"

#include <DirectXMath.h>
#include "core/application.h"
#include "core/core.h"
#include "core/input.h"
#include "core/layer.h"
#include "core/memory.h"
#include "core/timer.h"
#include "events/input_event.h"
#include "events/keyboard_event.h"
#include "render/window.h"
#include "tools/profiling.h"

namespace Sapfire {

	// Setting the Agility SDK parameters.
	extern "C" {
	__declspec(dllexport) extern const UINT D3D12SDKVersion = 614u;
	}
	extern "C" {
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\";
	}

	bool calculate_frame_stats(f32 delta, f32& out_fps);

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationCreationDesc& desc) : m_Name(desc.name), m_Minimized(false) {
		PROFILE_FUNCTION();
		input::InputSystem::init();
		s_Instance = this;
		fs::FileSystem::locate_root_directory();
		m_Window = stl::make_unique<Window>(
			mem::ENUM::Engine_Core,
			WindowParams{desc.width, desc.height, stl::string(desc.name), BIND_EVENT_FN(Application::on_event), desc.window_proc});
	}

	Application::~Application() { PROFILE_FUNCTION(); }

	void Application::run() {
		PROFILE_FUNCTION();
		Timer timer{};
		while (m_Running) {
			PROFILE_SCOPE("Run Loop");
			const f32 delta_time = timer.elapsed_millis() * 0.001f;
			timer.reset();
			m_Window->pump_messages();
			execute_main_thread_queue();
			input::InputSystem::update();
			if (!m_Minimized) {
				{
					PROFILE_SCOPE("Application event_loop");
					event_loop(delta_time);
				}
				{
					PROFILE_SCOPE("LayerStack on_update");
					for (auto* layer : m_LayerStack) {
						layer->on_update(delta_time);
					}
					for (auto* layer : m_LayerStack) {
						layer->on_render();
					}
				}
				/* f32 fps; */
				/* if (calculate_frame_stats(delta_time, fps)) { */
				/* 	CORE_INFO("FPS: {}", fps); */
				/* } */
			}
		}
	}

	bool calculate_frame_stats(f32 delta, f32& out_fps) {
		static int frame_count = 0;
		static f32 time_elapsed = 0.0f;
		frame_count++;
		time_elapsed += delta;
		if (time_elapsed >= FRAME_STATS_PERIOD) {
			out_fps = static_cast<f32>(frame_count) / time_elapsed;
			time_elapsed = 0.f;
			frame_count = 0;
			return true;
		}
		return false;
	}

	void Application::close() { m_Running = false; }

	void Application::push_layer(Layer* layer) {
		m_LayerStack.push_layer(layer);
		layer->on_attach();
	}

	void Application::push_overlay(Layer* layer) {
		m_LayerStack.push_overlay(layer);
		layer->on_attach();
	}

	void Application::on_event(Event& e) {
		PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		// @TODO bind window close and window resize
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::on_window_close));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::on_window_resize));
		dispatcher.dispatch<WindowResizeFinishedEvent>(BIND_EVENT_FN(Application::on_window_resize_finished));
		dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(Application::on_mouse_moved));
		dispatcher.dispatch<MouseButtonEvent>(BIND_EVENT_FN(Application::on_mouse_button));
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT_FN(Application::on_key_pressed));
		dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT_FN(Application::on_key_released));
		for (auto* layer : m_LayerStack) {
			if (e.Handled)
				break;
			layer->on_event(e);
		}
	}

	bool Application::on_mouse_button(MouseButtonEvent& e) {
		input::MouseState state;
		state.RMB = e.button() == MouseButton::RMB && e.is_down();
		state.LMB = e.button() == MouseButton::LMB && e.is_down();
		input::InputSystem::mouse_state(state);
		return true;
	}

	bool Application::on_mouse_moved(MouseMovedEvent& e) {
		input::InputSystem::mouse_position({e.x(), e.y()});
		return true;
	}

	bool Application::on_key_pressed(KeyPressedEvent& e) {
		input::InputSystem::keyboard_state(e.state());
		return true;
	}

	bool Application::on_key_released(KeyReleasedEvent& e) {
		input::InputSystem::keyboard_state(e.state());
		return true;
	}

	bool Application::on_window_close(WindowCloseEvent&) {
		close();
		return true;
	}

	bool Application::on_window_resize(WindowResizeEvent& e) {
		PROFILE_FUNCTION();
		m_ClientExtent = {e.width(), e.height()};
		return true;
	}

	bool Application::on_window_resize_finished(WindowResizeFinishedEvent&) { return true; }

	void Application::submit_to_main_thread(const std::function<void()>& func) {
		const stl::lock_guard<stl::mutex> lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.emplace_back(func);
	}

	void Application::execute_main_thread_queue() {
		const stl::lock_guard<stl::mutex> lock(m_MainThreadQueueMutex);
		for (auto& func : m_MainThreadQueue) {
			func();
		}
		m_MainThreadQueue.clear();
	}

	Application& Application::get() { return *s_Instance; }

} // namespace Sapfire
