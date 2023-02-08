#pragma once
#include "Sapfire/events/Event.h"

namespace Sapfire
{
	class Layer
	{
	public:
		inline Layer(const std::string& name = "Layer") : mName(name) {}
		inline virtual ~Layer() {};

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImguiRender() {}
		inline const std::string& GetName() const { return mName; }

	private:
		std::string mName;
	};
}