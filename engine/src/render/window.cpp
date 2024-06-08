#include "engpch.h"

#include <windowsx.h>
#include <winuser.h>
#include "events/application_event.h"
#include "events/input_event.h"
#include "events/keyboard_event.h"
#include "render/window.h"

namespace Sapfire {
	Window::Window(const WindowParams& params) :
		m_WindowExtent({params.width, params.height}), mf_EventCallback(params.callback), m_Resizing(false) {
		WNDCLASSEX window_class{0};
		window_class.cbSize = sizeof(WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
		window_class.lpszClassName = "SapfireApplicationClass";
		window_class.hInstance = nullptr;
		window_class.lpfnWndProc = params.window_proc == nullptr ? WindowProc : params.window_proc;
		RegisterClassEx(&window_class);
		RECT window_rect{0, 0, static_cast<LONG>(m_WindowExtent.width), static_cast<LONG>(m_WindowExtent.height)};
		AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
		m_Hwnd = CreateWindow(window_class.lpszClassName, params.name.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
							  window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, nullptr, nullptr, nullptr, this);
		if (m_Hwnd == nullptr) {
			CORE_CRITICAL("Failed to create window.");
			MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONERROR | MB_OK);
		}
		ShowWindow(m_Hwnd, SW_SHOWDEFAULT);
	}

	Window::~Window() { DestroyWindow(m_Hwnd); }

	void Window::pump_messages() {
		MSG msg{};
		if (PeekMessage(&msg, m_Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (message) {
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
			window->m_Resizing = true;
			return 0;
		case WM_EXITSIZEMOVE:
			{
				window->m_Resizing = false;
				WindowResizeFinishedEvent event{};
				window->event_callback(event);
				return 0;
			}
		case WM_SYSCOMMAND:
			{
				if (wParam == SC_MINIMIZE || wParam == SC_MAXIMIZE) {
					WindowResizeFinishedEvent event{};
					window->m_Minimized = wParam == SC_MINIMIZE;
					window->event_callback(event);
				}
			}
			break;
		case WM_SIZE:
			{
				WindowResizeEvent event(LOWORD(lParam), HIWORD(lParam));
				if (event.width() == 0 && event.height() == 0) {
					window->m_Minimized = true;
				} else {
					window->m_Minimized = false;
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

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
} // namespace Sapfire
