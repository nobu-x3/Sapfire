#include "engine/Actor.h"

class Asteroid : public Actor
{
      public:
	Asteroid(class Game *game);
	~Asteroid();
	void Shoot(Vector2 force);
	void UpdateActor(float deltaTime) override;
	inline class CircleColliderComponent *GetCollider() const { return mCollider; }

      private:
	class CircleColliderComponent *mCollider;
	class MovementComponent *mMovementComp;
	float mForce;
};
