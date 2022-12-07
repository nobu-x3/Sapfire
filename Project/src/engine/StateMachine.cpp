#include "StateMachine.h"
#include "Actor.h"
#include "State.h"
#include <SDL_log.h>
#include <cstring>

using namespace Patterns;

void StateMachine::RegisterState(State *state)
{
	if (mStateMap.find(state->GetName()) != mStateMap.end())
	{
		SDL_Log("State with the name %s already exists in the state map!", state->GetName());
		return;
	}

	mStateMap.emplace(state->GetName(), state);
}

void StateMachine::Update(float deltaTime)
{
	if (mCurrentState)
	{
		mCurrentState->Update(deltaTime);
	}
}

void StateMachine::ChangeState(const std::string &name)
{
	auto iter = mStateMap.find(name);
	if (iter == mStateMap.end())
	{
		SDL_Log("State with the name %s does not exist in the state map!", name.c_str());
		return;
	}

	mCurrentState->OnExit();
	mCurrentState = iter->second;
	mCurrentState->OnEnter();
}

void StateMachine::ProcessInput(const uint8_t *keyState)
{
	Component::ProcessInput(keyState);
}
