#include "CameraActor.h"
#include "Game.h"
#include "MovementComponent.h"
#include "Renderer.h"
#include "SDL2/SDL_scancode.h"

CameraActor::CameraActor(Game *game) : Actor(game)
{
	mMoveComp = new MovementComponent(this);
}

void CameraActor::ActorInput(const uint8_t *keys)
{
	float angularSpeed = 0.0f;
	float forwardSpeed = 0.0f;
	// wasd movement
	if (keys[SDL_SCANCODE_W])
	{
		forwardSpeed += 300.0f;
	}
	if (keys[SDL_SCANCODE_S])
	{
		forwardSpeed -= 300.0f;
	}
	if (keys[SDL_SCANCODE_A])
	{
		angularSpeed -= Math::TwoPi;
	}
	if (keys[SDL_SCANCODE_D])
	{
		angularSpeed += Math::TwoPi;
	}

	mMoveComp->SetForwardSpeed(forwardSpeed);
	mMoveComp->SetAngularSpeed(angularSpeed);
}

void CameraActor::UpdateActor(float deltaTime)
{
	// Compute new camera from this actor
	Vector3 cameraPos = GetPosition();
	Vector3 target = GetPosition() + GetForwardVector() * 100.0f;
	Vector3 up = Vector3::UnitZ;

	Matrix4 view = Matrix4::CreateLookAt(cameraPos, target, up);
	GetGame()->GetRenderer()->SetViewMatrix(view);
}
