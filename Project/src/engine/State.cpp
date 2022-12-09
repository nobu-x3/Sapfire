#include "State.h"
#include "AnimSpriteComponent.h"
#include "SDL2/SDL_log.h"
#include "StateMachine.h"

State::State(const char *name, StateMachine *sm, AnimSpriteComponent *animSpriteComp)
    : mName(name), mOwner(sm), mAnimSpriteComponent(animSpriteComp)
{
	sm->RegisterState(this);
}

void State::OnEnter()
{
	SDL_Log("STATE::ENTER::%s", mName);
	mAnimSpriteComponent->PlayAnimation(GetName());
}

void State::OnExit()
{
	SDL_Log("STATE::EXIT::%s", mName);
}
