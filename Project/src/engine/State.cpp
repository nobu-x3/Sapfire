#include "State.h"
#include "SDL2/SDL_log.h"
#include "StateMachine.h"

State::State(StateMachine *sm) : mOwner(sm)
{
}
