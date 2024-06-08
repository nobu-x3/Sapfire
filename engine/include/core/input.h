#pragma once

#include <DirectXMath.h>
#include "core/core.h"
#include "core/stl/unique_ptr.h"

namespace Sapfire::input {

	struct SFAPI MousePosition {
		s32 x = 0, y = 0;
	};

	struct SFAPI MouseState {
		bool RMB = false, LMB = false;
	};

	struct SFAPI InputComponent {
		InputComponent();
		DirectX::XMFLOAT4 input_axis;
		float sens_v = 0.002f;
		float sens_h = 0.002f;
		float mouse_delta_x = 0.f;
		float mouse_delta_y = 0.f;
		MouseState mouse_state{};
	};

	class SFAPI InputSystem {
	public:
		static void init();
		InputSystem() = default;
		InputSystem(const InputSystem&) = delete;
		InputSystem(InputSystem&&) = delete;
		InputSystem& operator=(const InputSystem&) = delete;
		InputSystem& operator=(InputSystem&&) = delete;
		static bool is_init();
		static void mouse_position(MousePosition pos);
		static void mouse_state(MouseState state);
		static void keyboard_state(u64 state);
		static void update();
		static void register_component(InputComponent& component);
		static MousePosition mouse_position();
		static MouseState mouse_state();
		static u64 keyboard_state();
		static bool is_key_down(s32 scan_code);

	private:
		static stl::unique_ptr<InputSystem> s_Instance;
		stl::vector<std::reference_wrapper<InputComponent>> m_InputComponents;
		MousePosition m_MousePosition{};
		MousePosition m_LastMousePosition{};
		MouseState m_MouseState{};
		MouseState m_LastMouseState{};
		u64 m_KeyboardState{};
	};

} // namespace Sapfire::input
