#include "IdleState.h"
#include "engine/StateMachine.h"
#include <SDL_scancode.h>

void IdleState::ProcessInput(const uint8_t *keyState)
{
	if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_D])
	{
		mOwner->ChangeState("move");
	}
}
