#pragma once
#include "Sapfire/events/Event.h"

namespace Sapfire
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : mName(name) {}
		virtual ~Layer() {};

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update(float deltaTime) {}
		virtual void on_event(Event& event) {}
		virtual void on_imgui_render() { PROFILE_FUNCTION(); }
		const std::string& get_name() const { return mName; }

	private:
		std::string mName;
	};
}