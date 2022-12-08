#pragma once
#include "engine/Actor.h"

class AIActor : public Actor
{
	public:
	AIActor(class Game *game);

	private:
	class StateMachine *mStateMachine;
	class AnimSpriteComponent *mAnimSpriteComponent;
};
