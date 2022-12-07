#include "State.h"
#include "SDL2/SDL_log.h"
using namespace Patterns;

void State::OnEnter()
{
	SDL_Log("STATEMACHINE::Entered State %s", GetName());
}

void State::OnExit()
{
	SDL_Log("STATEMACHINE::Exited State %s", GetName());
}
