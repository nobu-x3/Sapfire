#include "MovementComponent.h"
#include "Actor.h"
#include "Math.h"
#include "engine/Component.h"
MovementComponent::MovementComponent(Actor *owner, float mass, int updateOrder)
    : Component(owner, updateOrder), mMass(mass), mNetForce(Vector2::Zero), mVelocity(Vector2::Zero), mAngularSpeed(0)
{
}

void MovementComponent::Update(float deltaTime)
{
	auto acceleration = mNetForce * (1 / mMass);

	// Semi-implicit Euler Integration
	mVelocity += acceleration * deltaTime;
	auto position = mOwner->GetPosition();
	position += mVelocity * deltaTime;
	mOwner->SetPosition(position);

	if (!Math::NearZero(mAngularSpeed))
	{
		Quaternion rot = mOwner->GetRotation();
		float angle = mAngularSpeed * deltaTime;
		// for now rotate around up axis
		Quaternion inc(Vector3::UnitZ, angle);
		rot = Quaternion::Concatenate(rot, inc);
		mOwner->SetRotation(rot);
	}
}
