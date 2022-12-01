#include "Component.h"
class MovementComponent : public Component
{
      public:
	MovementComponent(class Actor *owner, int updateOrder = 10)
	    : Component(owner, updateOrder), mAngularSpeed(0), mForwardSpeed(0)
	{
	}

	void Update(float deltaTime) override;

	inline float GetForwardSpeed() const { return mForwardSpeed; }
	inline float GetAngularSpeed() const { return mAngularSpeed; }
	inline void SetForwardSpeed(float forwardSpeed) { mForwardSpeed = forwardSpeed; }
	inline void SetAngularSpeed(float angularSpeed) { mAngularSpeed = angularSpeed; }

      private:
	// controls rotation (radians/sec)
	float mAngularSpeed;
	// controls forwardSpeed (units/sec)
	float mForwardSpeed;
};
