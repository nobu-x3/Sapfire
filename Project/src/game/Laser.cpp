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
	mMovementComponent = new MovementComponent(this, 1.f);
	/* mMovementComponent->SetForwardSpeed(850.0f); */
	mSpriteComponent = new SpriteComponent(this);
	mSpriteComponent->SetTexture(game->LoadTexture("../Assets/Laser.png"));
	mRemovalTimer = 1.0f;
}

void Laser::UpdateActor(float deltaTime)
{
	mRemovalTimer -= deltaTime;
	if (mRemovalTimer <= 0.0f)
	{
		SetState(ActorState::EPendingRemoval);
	}
	for (auto roid : GetGame()->GetAsteroids())
	{
		if (CircleColliderComponent::Intersect(*mCollider, *(roid->GetCollider())))
		{
			// kill laser and Asteroid
			SetState(ActorState::EPendingRemoval);
			roid->SetState(ActorState::EPendingRemoval);
			break;
		}
	}
}

void Laser::Shoot(Vector2 force)
{
	mMovementComponent->AddForce(force);
}
