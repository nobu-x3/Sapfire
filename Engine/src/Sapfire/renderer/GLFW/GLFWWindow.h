#pragma once

#include "Sapfire/renderer/Window.h"
#include "Sapfire/renderer/RenderingContext.h"
#include <GLFW/glfw3.h>

namespace Sapfire
{
	class GLFWWindow : public Window
	{
	public:
		GLFWWindow(const WindowProperties& props);
		virtual ~GLFWWindow();
		// Inherited via Window
		virtual void OnUpdate() override;
		inline virtual unsigned int GetWidth() const override { return mData.Width; }
		inline virtual unsigned int GetHeight() const override { return mData.Height; }
		inline virtual void SetEventCallback(const EventCallback& callback) override { mData.EventCallback = callback; }
		virtual float GetTime() const override;
		void* GetNativeWindow() const override { return mWindow; }

	private:
		GLFWwindow* mWindow;
		Scope<RenderingContext> mContext;
		WindowData mData;

	};
}