#include "IdleState.h"
#include "engine/AnimSpriteComponent.h"
#include "engine/StateMachine.h"
IdleState::IdleState(StateMachine *sm, AnimSpriteComponent *sc) : State(sm), mAnimSpriteComp(sc)
{
}

void IdleState::OnEnter()
{
	mAnimSpriteComp->PlayAnimation(GetName());
}

void IdleState::Update(float deltaTime)
{
}

void IdleState::OnExit()
{
}
