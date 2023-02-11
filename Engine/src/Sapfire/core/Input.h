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
		static bool MouseButtonDown(int button);
		static bool MouseButtonUp(int button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}