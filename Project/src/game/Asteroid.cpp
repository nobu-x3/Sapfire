#include "Asteroid.h"
#include "engine/CircleColliderComponent.h"
#include "engine/Game.h"
#include "engine/MovementComponent.h"
#include "engine/Random.h"
#include "engine/SpriteComponent.h"
Asteroid::Asteroid(Game *game) : Actor(game)
{
	Vector2 randPos = Random::GetVector(Vector2::Zero, Vector2(1024.f, 768.f));
	SetPosition(randPos);
	SetRotation(Random::GetFloatRange(0.f, Math::TwoPi));
	SpriteComponent *sc = new SpriteComponent(this);
	sc->SetTexture(game->LoadTexture("../Assets/Asteroid.png"));
	mMovementComp = new MovementComponent(this, 1.f);
	mForce = Random::GetFloatRange(0.f, 150.0f);
	mMovementComp->AddForce(GetForwardVector() * mForce);
	mCollider = new CircleColliderComponent(this);
	mCollider->SetRadius(40.0f);
}

Asteroid::~Asteroid()
{
	GetGame()->RemoveAsteroid(this);
}

void Asteroid::UpdateActor(float deltaTime)
{
	mMovementComp->SetNetForce(Vector2::Zero);
	mMovementComp->AddForce(GetForwardVector() * mForce);
}
