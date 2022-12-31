#pragma once
#include "Component.h"
#include "engine/Math.h"
class MovementComponent : public Component
{
      public:
      MovementComponent(class Actor *owner, int updateOrder = 10)
	  : Component(owner, updateOrder), mAngularSpeed(0), mNetForce(Vector3::Zero), mMass(0),
	    mVelocity(Vector3::Zero)
      {
	}

	MovementComponent(class Actor *owner, float mass, int updateOrder = 10);

	void Update(float deltaTime) override;
	/* inline float GetForwardSpeed() const { return mForwardSpeed; } */
	inline float GetAngularSpeed() const { return mAngularSpeed; }
	inline float GetMass() const { return mMass; }
	inline Vector3 GetNetForce() const { return mNetForce; }
	/* inline void SetForwardSpeed(float forwardSpeed) { mForwardSpeed = forwardSpeed; } */
	inline void SetAngularSpeed(float angularSpeed) { mAngularSpeed = angularSpeed; }
	inline void SetMass(float mass) { mMass = mass; }
	inline void SetNetForce(Vector3 force) { mNetForce = force; }
	inline void AddForce(Vector3 force) { mNetForce += force; }

	private:
	// controls rotation (radians/sec)
	float mAngularSpeed;
	// controls forwardSpeed (units/sec)
	float mForwardSpeed;

	float mMass;

	Vector3 mNetForce;
	Vector3 mVelocity;
};
