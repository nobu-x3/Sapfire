#include "Laser.h"
#include "Asteroid.h"
#include "engine/Actor.h"
#include "engine/CircleColliderComponent.h"
#include "engine/Game.h"
#include "engine/MovementComponent.h"
#include "engine/SpriteComponent.h"
Laser::Laser(Game *game) : Actor(game)
{
	mCollider = new CircleColliderComponent(this);
	mCollider->SetRadius(30.f);
	mMovementComponent = new MovementComponent(this);
	mMovementComponent->SetForwardSpeed(300.0f);
	mSpriteComponent = new SpriteComponent(this);
	mSpriteComponent->SetTexture(game->LoadTexture("../Assets/Laser.png"));
	mRemovalTimer = 1.0f;
}

void Laser::UpdateActor(float deltaTime)
{
	mRemovalTimer -= deltaTime;
	if (mRemovalTimer <= 0.0f)
	{
		SetState(State::EPendingRemoval);
	}
	for (auto roid : GetGame()->GetAsteroids())
	{
		if (CircleColliderComponent::Intersect(*mCollider, *(roid->GetCollider())))
		{
			// kill laser and Asteroid
			SetState(State::EPendingRemoval);
			roid->SetState(State::EPendingRemoval);
			break;
		}
	}
}
