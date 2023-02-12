#pragma once

#include "Sapfire/core/Layer.h"

namespace Sapfire
{
	class ImguiLayer : public Layer
	{
	public:
		ImguiLayer();
		~ImguiLayer();
		virtual void on_attach() override;
		virtual void on_detach() override;
		virtual void on_event(Event& event) override;
		void SetBlockEvents(bool val) { mBlockEvents = val; }
		void begin();
		void end();
	private:
		static void set_dark_theme();

	private:
		bool mBlockEvents = true;
	};
}