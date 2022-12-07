#include "Component.h"
class MovementComponent : public Component
{
      public:
	MovementComponent(class Actor *owner, int updateOrder = 10)
	    : Component(owner, updateOrder), mAngularSpeed(0), mNetForce(Vector2::Zero), mMass(0),
	      mVelocity(Vector2::Zero)
	{
	}

	MovementComponent(class Actor *owner, float mass, int updateOrder = 10);

	void Update(float deltaTime) override;
	/* inline float GetForwardSpeed() const { return mForwardSpeed; } */
	inline float GetAngularSpeed() const { return mAngularSpeed; }
	inline float GetMass() const { return mMass; }
	inline Vector2 GetNetForce() const { return mNetForce; }
	/* inline void SetForwardSpeed(float forwardSpeed) { mForwardSpeed = forwardSpeed; } */
	inline void SetAngularSpeed(float angularSpeed) { mAngularSpeed = angularSpeed; }
	inline void SetMass(float mass) { mMass = mass; }
	inline void SetNetForce(Vector2 force) { mNetForce = force; }
	inline void AddForce(Vector2 force) { mNetForce += force; }

      private:
	// controls rotation (radians/sec)
	float mAngularSpeed;
	// controls forwardSpeed (units/sec)
	/* float mForwardSpeed; */

	float mMass;

	Vector2 mNetForce;
	Vector2 mVelocity;
};
