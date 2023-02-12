#pragma once
#include "KeyCodes.h"

namespace Sapfire
{
	class Input
	{
	public:
		static bool key_pressed(KeyCode keycode);
		static bool key_released(KeyCode keycode);
		static bool mouse_button_down(MouseButton button);
		static bool mouse_button_up(MouseButton button);
		static std::pair<float, float> get_mouse_position();
		static float get_mouse_x();
		static float get_mouse_y();
	};
}