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
		virtual void on_update() override;
		virtual unsigned int get_width() const override { return mData.Width; }
		virtual unsigned int get_height() const override { return mData.Height; }
		virtual void set_event_callback(const EventCallback& callback) override { mData.EventCallback = callback; }
		virtual float get_time() const override;
		void* get_native_window() const override { return mWindow; }

	private:
		GLFWwindow* mWindow;
		Scope<RenderingContext> mContext;
		WindowData mData;

	};
}