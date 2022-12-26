#pragma once
#include "MovementComponent.h"
#include <cstdint>

class InputComponent : public MovementComponent
{
      public:
	InputComponent(class Actor *owner, float mass, int updateOrder = 10);

	void ProcessInput(const uint8_t *keyState) override;

	inline float GetMaxForwardSpeed() { return mMaxForwardSpeed; }
	inline void SetMaxForwardSpeed(float speed) { mMaxForwardSpeed = speed; }
	inline float GetMaxAngularSpeed() { return mMaxAngularSpeed; }
	inline void SetMaxAngularSpeed(float speed) { mMaxAngularSpeed = speed; }

	inline uint8_t GetForwardKey() { return mForwardKey; }
	inline void SetForwardKey(uint8_t key) { mForwardKey = key; }
	inline uint8_t GetBackwardKey() { return mBackwardKey; }
	inline void SetBackwardKey(uint8_t key) { mBackwardKey = key; }
	inline uint8_t GetClockwiseKey() { return mClockwiseKey; }
	inline void SetClockwiseKey(uint8_t key) { mClockwiseKey = key; }
	inline uint8_t GetCounterClockwiseKey() { return mCounterClockwiseKey; }
	inline void SetCounterClockwiseKey(uint8_t key) { mCounterClockwiseKey = key; }

      private:
	float mMaxForwardSpeed;
	float mMaxAngularSpeed;

	uint8_t mForwardKey;
	uint8_t mBackwardKey;
	uint8_t mClockwiseKey;
	uint8_t mCounterClockwiseKey;
};
