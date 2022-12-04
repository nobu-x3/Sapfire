#include "InputComponent.h"
#include "engine/Actor.h"
#include <SDL_log.h>
#include <SDL_scancode.h>

InputComponent::InputComponent(Actor *owner, int updateOrder)
    : MovementComponent(owner, updateOrder), mForwardKey(SDL_SCANCODE_W), mBackwardKey(SDL_SCANCODE_S),
      mClockwiseKey(SDL_SCANCODE_D), mCounterClockwiseKey(SDL_SCANCODE_A), mMaxForwardSpeed(0), mMaxAngularSpeed(0)
{
}

void InputComponent::ProcessInput(const uint8_t *keyState)
{
	float forwardSpeed = 0.0f;
	if (keyState[mForwardKey])
	{
		forwardSpeed += mMaxForwardSpeed;
	}
	if (keyState[mBackwardKey])
		forwardSpeed -= mMaxForwardSpeed;
	SetForwardSpeed(forwardSpeed);

	float angularSpeed = 0.0f;
	if (keyState[mClockwiseKey])
		angularSpeed += mMaxAngularSpeed;
	if (keyState[mCounterClockwiseKey])
		angularSpeed -= mMaxAngularSpeed;
	SetAngularSpeed(angularSpeed);

}
