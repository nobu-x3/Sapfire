#pragma once
#include "engine/Actor.h"
#include <SDL_scancode.h>
#include <cstdint>

class AIActor : public Actor
{
	public:
	AIActor(class Game *game);

	private:
	class StateMachine *mStateMachine;
	class AnimSpriteComponent *mAnimSpriteComponent;
	class InputComponent *mInputComponent;
};
