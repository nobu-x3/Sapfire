#include "engine/Actor.h"

class Asteroid : public Actor
{
      public:
	Asteroid(class Game *game);
	~Asteroid();
	inline class CircleColliderComponent *GetCollider() const { return mCollider; }

      private:
	class CircleColliderComponent *mCollider;
};
