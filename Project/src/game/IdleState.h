#pragma once
#include "engine/State.h"

class IdleState : public State
{
	public:
	IdleState(const char *name, class StateMachine *owner, class AnimSpriteComponent *animSpriteComp)
	    : State(name, owner, animSpriteComp)
	{
	}
	void ProcessInput(const uint8_t *keyState) override;
};
