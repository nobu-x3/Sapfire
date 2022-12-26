#include "Ship.h"
#include "Asteroid.h"
#include "Laser.h"
#include "engine/CircleColliderComponent.h"
#include "engine/Game.h"
#include "engine/InputComponent.h"
#include "engine/SpriteComponent.h"
#include <SDL_scancode.h>
#include <vector>

Ship::Ship(Game *game) : Actor(game)
{
	// Create an animated sprite component
	mSpriteComp = new SpriteComponent(this);
	mSpriteComp->SetTexture(game->LoadTexture("../Assets/Ship.png"));
	mInputComponent = new InputComponent(this, 1.0f);
	mInputComponent->SetMaxAngularSpeed(5.0f);
	mInputComponent->SetMaxForwardSpeed(210.0f);

	mLaserCooldown = 0.5f;

	mCollisionComponent = new CircleColliderComponent(this);
	mCollisionComponent->SetRadius(48.f);
}

void Ship::ActorInput(const uint8_t *state)
{
	if (state[SDL_SCANCODE_SPACE] && mLaserCooldown <= 0.0f)
	{
		/* mAnimComponent->PlayAnimation("anim_character"); */
		Laser *laser = new Laser(GetGame()); // NOTE doesnt leak memory but is inefficient cuz heap allocations,
						     // use object pooling instead itself unless hit asteroid
		laser->SetPosition(GetPosition());
		laser->SetRotation(GetRotation());
		laser->Shoot(GetForwardVector() * 2000.f);
		mLaserCooldown = 0.5f;
	}
}

void Ship::UpdateActor(float deltaTime)
{
	if (mLaserCooldown > 0)
		mLaserCooldown -= deltaTime;

	for (auto roid : GetGame()->GetAsteroids())
	{
		if (CircleColliderComponent::Intersect(*mCollisionComponent, *(roid->GetCollider())))
		{
			roid->SetState(ActorState::EPendingRemoval);
			SetState(ActorState::EPendingRemoval);
			GetGame()->NotifyShipDeath();
		}
	}
}
