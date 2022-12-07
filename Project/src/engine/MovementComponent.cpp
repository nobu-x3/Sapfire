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
	auto acceleration = mNetForce / mMass;

	// Semi-implicit Euler Integration
	mVelocity += acceleration * deltaTime;
	auto position = mOwner->GetPosition();
	position += mVelocity * deltaTime;
	mOwner->SetPosition(position);

	if (!Math::NearZero(mAngularSpeed))
	{
		float rot = mOwner->GetRotation();
		rot += mAngularSpeed * deltaTime;
		mOwner->SetRotation(rot);
	}

	/* if (!Math::NearZero(mForwardSpeed)) */
	/* { */
	/* 	Vector2 pos = mOwner->GetPosition(); */
	/* 	pos += mOwner->GetForwardVector() * mForwardSpeed * deltaTime; */
	/* 	mOwner->SetPosition(pos); */
	/* } */
}
