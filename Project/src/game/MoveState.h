#pragma once
#include "engine/State.h"

class MoveState : public State
{
	public:
	MoveState(const char *name, class StateMachine *sm, class AnimSpriteComponent *sc, class MovementComponent *mc);

	inline void SetDirection(int dir) { mDirection = dir; }
	void Update(float deltaTime) override;

	void ProcessInput(const uint8_t *keyState) override;

	private:
	int mDirection;
	class MovementComponent *mMovementComp;
};
