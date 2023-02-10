#pragma once
#include <bitset>
#include "KeyCodes.h"

namespace Sapfire
{
	class Input
	{ 
		friend class Window;
	public:
		inline static void Init(Input* instance) { sInstance = instance; }
		inline static void Deinit() { delete sInstance; }
		inline static bool KeyPressed(KeyCode keycode) { return sInstance->KeyPressed_Impl(keycode); }
		inline static bool KeyReleased(KeyCode keycode) { return sInstance->KeyReleased_Impl(keycode); }
		inline static bool MouseButtonPressed(int button) { return sInstance->MouseButtonPressed_Impl(button); }
		inline static bool MouseButtonDown(int button) { return sInstance->MouseButtonDown_Impl(button); }
		inline static bool MouseButtonUp(int button) { return sInstance->MouseButtonUp_Impl(button); }
		inline static float GetMouseX() { return sInstance->GetMouseX_Impl(); }
		inline static float GetMouseY() { return sInstance->GetMouseY_Impl(); }
		inline static std::pair<float, float> GetMousePosition() { return sInstance->GetMousePosition_Impl(); }
		
	protected:
		virtual bool KeyPressed_Impl(KeyCode keycode) = 0;
		virtual bool KeyReleased_Impl(KeyCode keycode) = 0;
		virtual bool MouseButtonPressed_Impl(int button) = 0;
		virtual bool MouseButtonDown_Impl(int button) = 0;
		virtual bool MouseButtonUp_Impl(int button) = 0;
		virtual float GetMouseX_Impl() = 0;
		virtual float GetMouseY_Impl() = 0;
		virtual std::pair<float, float> GetMousePosition_Impl() = 0;
		
	private:
		static Input* sInstance;
	};
}