#include "Sapfire.h"

#include <windowsx.h>
#include "core/application.h"
#include "core/entry.h"
#include "imgui.h"
#include "sapling_layer.h"

using namespace Sapfire;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (msg) {
	case WM_CREATE:
		{
			auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
	case WM_CLOSE:
		{
			WindowCloseEvent event;
			window->event_callback(event);
			return 0;
		}
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	case WM_ENTERSIZEMOVE:
		window->is_resizing(true);
		return 0;
	case WM_EXITSIZEMOVE:
		{
			window->is_resizing(false);
			WindowResizeFinishedEvent event{};
			window->event_callback(event);
			return 0;
		}
	case WM_SYSCOMMAND:
		{
			if (wParam == SC_MINIMIZE || wParam == SC_MAXIMIZE) {
				WindowResizeFinishedEvent event{};
				window->is_minimized(wParam == SC_MINIMIZE);
				window->event_callback(event);
			}
		}
		break;
	case WM_SIZE:
		{
			WindowResizeEvent event(LOWORD(lParam), HIWORD(lParam));
			if (event.width() == 0 && event.height() == 0) {
				window->is_minimized(true);
			} else {
				window->is_minimized(false);
			}
			window->event_callback(event);
			return 0;
		}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			MouseButton button;
			switch (wParam) {
			case MK_LBUTTON:
				button = MouseButton::LMB;
				break;
			case MK_RBUTTON:
				button = MouseButton::RMB;
				break;
			case MK_MBUTTON:
				button = MouseButton::MMB;
				break;
			}
			MouseButtonEvent event(button, true);
			window->event_callback(event);
			return 0;
		}
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		{
			MouseButton button;
			switch (wParam) {
			case MK_LBUTTON:
				button = MouseButton::LMB;
				break;
			case MK_RBUTTON:
				button = MouseButton::RMB;
				break;
			case MK_MBUTTON:
				button = MouseButton::MMB;
				break;
			}
			MouseButtonEvent event(button, false);
			window->event_callback(event);
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			MouseMovedEvent event{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			window->event_callback(event);
			return 0;
		}
	case WM_KEYDOWN:
		{
			KeyPressedEvent event(wParam);
			window->event_callback(event);
			return 0;
		}
	case WM_KEYUP:
		{
			KeyReleasedEvent event(wParam);
			window->event_callback(event);
			return 0;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

class SaplingApp final : public Sapfire::Application {
public:
	SaplingApp(const ApplicationCreationDesc& desc) :
		Sapfire::Application(desc) {
            push_layer(new SaplingLayer());
	}

	~SaplingApp() {
	}

protected:
	void event_loop(f32 delta_time) override;
	bool on_window_resize_finished(WindowResizeFinishedEvent& e) override;
	bool on_window_resize(WindowResizeEvent& e) override;

};

Sapfire::Application* Sapfire::create_application() {
	return new SaplingApp({
		.name = "Sapling",
		.width = 800,
		.height = 600,
		.window_proc = WindowProc,
	});
}

void SaplingApp::event_loop(f32 delta_time) {
	/* m_GameContext.update(delta_time); */
	/* m_GameContext.render(); */
}

bool SaplingApp::on_window_resize_finished(WindowResizeFinishedEvent& e) {
	/* bool return_val = Application::on_window_resize_finished(e); */
	/* if (!m_Window->is_minimized()) */
	/* 	m_GameContext.on_window_resize(); */
	/* m_GameContext.resize_depth_texture(); */
	/* return return_val; */
    return false;
}

bool SaplingApp::on_window_resize(WindowResizeEvent& e) {
	bool return_val = Application::on_window_resize(e);
	/* if (m_Window && !m_Window->is_minimized() && !m_Window->is_resizing()) */
	/* 	m_GameContext.on_window_resize(); */
	/* if (!m_Window->is_resizing() && !m_Window->is_minimized()) { */
	/* 	m_GameContext.resize_depth_texture(); */
	/* } */
	/* return return_val; */
    return false;
}
