#pragma once
#include "engine/Actor.h"
#include <cstdint>
class Ship : public Actor
{
      public:
	Ship(class Game *game);
	void UpdateActor(float deltaTime) override;
	void ProcessKeyboard(const uint8_t *state);
	inline float GetRightSpeed() const { return mRightSpeed; }
	inline float GetDownSpeed() const { return mDownSpeed; }

      private:
	float mRightSpeed;
	class AnimSpriteComponent *mAnimComponent;
	float mDownSpeed;
};
