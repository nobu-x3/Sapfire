#pragma once

#include <functional>
#include "core/core.h"
#include "events/event.h"

#ifdef SF_PLATFORM_WINDOWS
#include <windows.h>
#include <wrl.h>
#endif

namespace Sapfire {

	using EventCallbackFn = std::function<void(Event&)>;

	struct SFAPI WindowParams {
		u64 width, height;
		stl::string name;
		EventCallbackFn callback;
		LRESULT (*window_proc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = nullptr;
	};

	struct SFAPI WindowExtent {
		u64 width, height;
	};

	class SFAPI Window {
	public:
		Window(const WindowParams& params);
		~Window();
		void pump_messages();
		void event_callback(Event& event) { mf_EventCallback(event); }
		void is_resizing(bool val) { m_Resizing = val; }
		bool is_resizing() const { return m_Resizing; }
		void is_minimized(bool val) { m_Minimized = val; }
		bool is_minimized() const { return m_Minimized; }
#ifdef SF_PLATFORM_WINDOWS
		HWND handle() { return m_Hwnd; }
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

	private:
		WindowExtent m_WindowExtent;
		EventCallbackFn mf_EventCallback;
		bool m_Resizing = false;
		bool m_Minimized = false;
#ifdef SF_PLATFORM_WINDOWS
		HWND m_Hwnd;
#endif
	};
} // namespace Sapfire
