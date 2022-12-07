#pragma once
#include "engine/Actor.h"
#include <cstdint>
class Ship : public Actor
{
      public:
	Ship(class Game *game);
	void ActorInput(const uint8_t *state) override;
	void UpdateActor(float deltaTime) override;

      private:
	class AnimSpriteComponent *mAnimComponent;
	class InputComponent *mInputComponent;
	class CircleColliderComponent *mCollisionComponent;
	float mLaserCooldown;
};
