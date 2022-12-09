#include "MoveState.h"
#include "engine/AnimSpriteComponent.h"
#include "engine/MovementComponent.h"
#include "engine/StateMachine.h"
#include <SDL_scancode.h>

MoveState::MoveState(const char *name, StateMachine *sm, AnimSpriteComponent *sc, MovementComponent *mc)
    : State(name, sm, sc), mMovementComp(mc)
{
}

void MoveState::Update(float deltaTime)
{
}

void MoveState::ProcessInput(const uint8_t *keyState)
{
	if (!keyState[SDL_SCANCODE_A] && !keyState[SDL_SCANCODE_D])
	{
		mOwner->ChangeState("idle");
	}
}
