#pragma once
#include <bitset>
#include "KeyCodes.h"

namespace Sapfire
{
	class Input
	{ 
	public:
		static bool KeyPressed(KeyCode keycode);
		static bool KeyReleased(KeyCode keycode);
		static bool MouseButtonDown(MouseButton button);
		static bool MouseButtonUp(MouseButton button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}