#pragma once

#include "Sapfire/core/Layer.h"

namespace Sapfire
{
	class ImguiLayer : public Layer
	{
	public:
		ImguiLayer();
		~ImguiLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;
		inline void SetBlockEvents(bool val) { mBlockEvents = val; }
		void Begin();
		void End();
	private:
		void SetDarkTheme();

	private:
		bool mBlockEvents = true;
	};
}