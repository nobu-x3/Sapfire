#include "Asteroid.h"
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
	MovementComponent *mc = new MovementComponent(this);
	mc->SetForwardSpeed(150.0f);
}
