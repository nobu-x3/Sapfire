#include "engpch.h"

#include <DirectXMath.h>
#include <windows.h>
#include <winuser.h>
#include "core/core.h"
#include "core/input.h"
#include "tools/profiling.h"

namespace Sapfire::input {

	constexpr f32 MOUSE_THRESHOLD = 0.001f;

	using namespace DirectX;

#define abs(x) (x >= 0 ? x : -x)

	stl::unique_ptr<InputSystem> InputSystem::s_Instance{nullptr};

	InputComponent::InputComponent() {
		if (!InputSystem::is_init()) {
			InputSystem::init();
		}
		InputSystem::register_component(*this);
	}

	void InputSystem::init() {
		if (!s_Instance)
			s_Instance = stl::make_unique<InputSystem>(mem::ENUM::Engine_Input);
	}

	bool InputSystem::is_init() { return s_Instance != nullptr; }

	void InputSystem::register_component(InputComponent& component) { s_Instance->m_InputComponents.push_back(component); }

	void InputSystem::mouse_position(MousePosition pos) {
		s_Instance->m_LastMousePosition = s_Instance->m_MousePosition;
		s_Instance->m_MousePosition = pos;
	}

	void InputSystem::mouse_state(MouseState state) {
		s_Instance->m_LastMouseState = s_Instance->m_MouseState;
		s_Instance->m_MouseState = state;
	}

	void InputSystem::keyboard_state(u64 state) { s_Instance->m_KeyboardState |= state; }

	MousePosition InputSystem::mouse_position() { return s_Instance->m_MousePosition; }

	MouseState InputSystem::mouse_state() { return s_Instance->m_MouseState; }

	u64 InputSystem::keyboard_state() { return s_Instance->m_KeyboardState; }

	bool InputSystem::is_key_down(s32 scan_code) { return GetAsyncKeyState(scan_code); }

	void InputSystem::update() {
		PROFILE_FUNCTION();
		for (auto& comp : s_Instance->m_InputComponents) {
			comp.get().mouse_state = s_Instance->m_MouseState;
			comp.get().mouse_delta_x = (s_Instance->m_MousePosition.x - s_Instance->m_LastMousePosition.x) * comp.get().sens_h;
			if (abs(comp.get().mouse_delta_x) < MOUSE_THRESHOLD) {
				comp.get().mouse_delta_x = 0.f;
			}
			comp.get().mouse_delta_y = (s_Instance->m_MousePosition.y - s_Instance->m_LastMousePosition.y) * comp.get().sens_v;
			if (abs(comp.get().mouse_delta_y) < MOUSE_THRESHOLD) {
				comp.get().mouse_delta_y = 0.f;
			}
			XMFLOAT4 input_axis{0.f, 0.f, 0.f, 0.f};
			if (is_key_down(VK_UP)) {
				input_axis.x += 1;
			}
			if (is_key_down(VK_DOWN)) {
				input_axis.x -= 1;
			}
			if (is_key_down(VK_RIGHT)) {
				input_axis.y += 1;
			}
			if (is_key_down(VK_LEFT)) {
				input_axis.y -= 1;
			}
			comp.get().input_axis = input_axis;
		}
		mouse_position(s_Instance->m_MousePosition);
	}
} // namespace Sapfire::input
