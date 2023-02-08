#pragma once

#include "Sapfire/core/Input.h"
#include "glfw/glfw3.h"

namespace Sapfire
{
	class GLFWInput : public Input
	{
	protected:
		// Inherited via Input
		virtual bool KeyPressed_Impl(int keycode) override;
		virtual bool KeyDown_Impl(int keycode) override;
		virtual bool KeyUp_Impl(int keycode) override;
		virtual bool MouseButtonPressed_Impl(int button) override;
		virtual bool MouseButtonDown_Impl(int button) override;
		virtual bool MouseButtonUp_Impl(int button) override;
		virtual std::pair<float, float> GetMousePosition_Impl() override;
		virtual float GetMouseX_Impl() override;
		virtual float GetMouseY_Impl() override;
		// Inherited via Input
	};
}