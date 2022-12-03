#include "MovementComponent.h"
#include "Math.h"

void MovementComponent::Update(float deltaTime)
{
	if (!Math::NearZero(mAngularSpeed))
	{
		float rot = mOwner->GetRotation();
		rot += mAngularSpeed * deltaTime;
		mOwner->SetRotation(rot);
	}

	if (!Math::NearZero(mForwardSpeed))
	{
		Vector2 pos = mOwner->GetPosition();
		pos += mOwner->GetForwardVector() * mForwardSpeed * deltaTime;
		mOwner->SetPosition(pos);
	}
}
