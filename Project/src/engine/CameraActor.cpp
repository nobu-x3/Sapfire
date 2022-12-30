#include "CameraActor.h"
#include "Game.h"
#include "MovementComponent.h"
#include "Renderer.h"
#include "SDL2/SDL_scancode.h"

CameraActor::CameraActor(Game *game) : Actor(game)
{
	mMoveComp = new MovementComponent(this);
}

void CameraActor::UpdateActor(float deltaTime)
{
	Actor::UpdateActor(deltaTime);

	// Compute new camera from this actor
	Vector3 cameraPos = GetPosition();
	Vector3 target = GetPosition() + GetForwardVector() * 100.0f;
	Vector3 up = Vector3::UnitZ;

	Matrix4 view = Matrix4::CreateLookAt(cameraPos, target, up);
	GetGame()->GetRenderer()->SetViewMatrix(view);
}

void CameraActor::ActorInput(const uint8_t *keys)
{
	mMoveComp->SetNetForce(Vector3::Zero);
	float angularSpeed = 0.0f;
	// wasd movement
	if (keys[SDL_SCANCODE_W])
	{
		mMoveComp->AddForce(GetForwardVector() * 300.f);
	}
	if (keys[SDL_SCANCODE_S])
	{
		mMoveComp->AddForce(GetForwardVector() * -300.f);
	}
	if (keys[SDL_SCANCODE_A])
	{
		angularSpeed -= Math::TwoPi;
	}
	if (keys[SDL_SCANCODE_D])
	{
		angularSpeed += Math::TwoPi;
	}

	mMoveComp->SetAngularSpeed(angularSpeed);
}
