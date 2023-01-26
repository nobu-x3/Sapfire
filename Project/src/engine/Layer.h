#pragma once
#include "engine/events/Event.h"

class Layer
{
	public:
	inline Layer(const std::string &name = "Layer") : mName(name) {}
	inline virtual ~Layer(){};

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(float deltaTime) {}
	virtual void OnEvent(Event &event) {}
	inline const std::string &GetName() const { return mName; }

	private:
	std::string mName;
};
