#include "InputComponent.h"
#include "engine/Actor.h"
#include <SDL_log.h>
#include <SDL_scancode.h>

InputComponent::InputComponent(Actor *owner, float mass, int updateOrder)
    : MovementComponent(owner, mass, updateOrder), mForwardKey(SDL_SCANCODE_W), mBackwardKey(SDL_SCANCODE_S),
      mClockwiseKey(SDL_SCANCODE_D), mCounterClockwiseKey(SDL_SCANCODE_A), mMaxForwardSpeed(0), mMaxAngularSpeed(0)
{
}

void InputComponent::ProcessInput(const uint8_t *keyState)
{
	SetNetForce(Vector3::Zero);
	if (keyState[mForwardKey])
	{
		AddForce(mOwner->GetForwardVector() * mMaxForwardSpeed);
	}
	if (keyState[mBackwardKey])
		AddForce(mOwner->GetForwardVector() * -mMaxForwardSpeed);

	float angularSpeed = 0.0f;
	if (keyState[mClockwiseKey])
		angularSpeed += mMaxAngularSpeed;
	if (keyState[mCounterClockwiseKey])
		angularSpeed -= mMaxAngularSpeed;
	SetAngularSpeed(angularSpeed);

}
