#include "engine/Actor.h"

class Laser : public Actor
{
	public:
	Laser(class Game *game);
	void UpdateActor(float deltaTime) override;
	inline class CircleColliderComponent *GetCollider() const { return mCollider; }
	inline void SetRemovalTimer(float time) { mRemovalTimer = time; }
	void Shoot(Vector2 force);

	private:
	class CircleColliderComponent *mCollider;
	class MovementComponent *mMovementComponent;
	class SpriteComponent *mSpriteComponent;
	float mRemovalTimer;
};
