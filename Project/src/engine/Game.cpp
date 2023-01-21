#include "Game.h"
#include "BGSpriteComponent.h"
#include "CameraActor.h"
#include "GL/glew.h"
#include "Mesh.h"
#include "PlaneActor.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "VertexArray.h"
#include "engine/Math.h"
#include "engine/MeshComponent.h"
#include "engine/Texture.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Shader.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_timer.h>
#include <SDL_video.h>

Game::Game()
{
	mIsRunning = true;
	mTicksCount = 0;
}

Game::~Game()
{
	while (!mActors.empty())
	{
		delete mActors.back();
	}
}

void Game::LoadData()
{
	Actor *cube1 = new Actor(this);
	cube1->SetPosition(Vector3(200.f, 75.f, 0.f));
	cube1->SetScale(100.f);
	Quaternion q(Vector3::UnitY, -Math::PiOver2);
	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::Pi + Math::Pi / 4.f));
	cube1->SetRotation(q);
	/* MeshComponent *mc = new MeshComponent(cube1); */
	/* mc->SetMesh(mRenderer->GetMesh("../Assets/Cube.sfmesh")); */

	Actor *cube2 = new Actor(this);
	cube2->SetPosition(cube1->GetPosition() + Vector3(0.f, 400.f, 100.f));
	cube2->SetScale(100.f);
	cube2->SetRotation(q);
	MeshComponent *mc2 = new MeshComponent(cube2);
	mc2->SetMesh(mRenderer->GetMesh("../Assets/Cube2.sfmesh"));

	const float start = -1250.f;
	const float size = 250.f;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			cube1 = new PlaneActor(this);
			cube1->SetPosition(Vector3(start + i * size, start + j * size, -100.f));
		}
	}

	// Left/right walls
	q = Quaternion(Vector3::UnitX, Math::PiOver2);
	for (int i = 0; i < 10; i++)
	{
		cube1 = new PlaneActor(this);
		cube1->SetPosition(Vector3(start + i * size, start - size, 0.f));
		cube1->SetRotation(q);

		cube1 = new PlaneActor(this);
		cube1->SetPosition(Vector3(start + i * size, -start + size, 0.f));
		cube1->SetRotation(q);
	}

	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::PiOver2));
	// Forward/back walls
	for (int i = 0; i < 10; i++)
	{
		cube1 = new PlaneActor(this);
		cube1->SetPosition(Vector3(start - size, start + i * size, 0.f));
		cube1->SetRotation(q);

		cube1 = new PlaneActor(this);
		cube1->SetPosition(Vector3(-start + size, start + i * size, 0.f));
		cube1->SetRotation(q);
	}
	// Setup lights
	mRenderer->SetAmbientLight(Vector3(0.1f, 0.1f, 0.1f));
	/* DirectionalLight &dir = mRenderer->GetDirectionalLight(); */
	/* dir.mDirection = Vector3(0.f, -0.707f, -0.707f); */
	/* dir.mDiffuseColor = Vector3(1.f, 1.f, 1.f); */
	/* dir.mSpecColor = Vector3(0.5f, 1.f, 0.5f); */

	std::array<PointLight, 4> &pointLights = mRenderer->GetPointLights();
	pointLights[0].mPosition = Vector3(0.f, 200.f, 0.f);
	pointLights[0].mDiffuseColor = Vector3(1.f, 1.f, 1.f);
	pointLights[0].mSpecColor = Vector3(1.f, 0.f, 0.f);
	pointLights[0].mIntensity = 400.f;
	pointLights[0].mRadius = 600.f;

	mCameraActor = new CameraActor(this);
	mCameraActor->SetPosition(Vector3(10.f, 10.f, 0.f));

	/* SDL_Log("%f %f %f", mCameraActor->GetPosition().x, mCameraActor->GetPosition().y, */
	/* 	mCameraActor->GetPosition().z); */
}

void Game::UnloadData()
{
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	while (!mActors.empty())
	{
		delete mActors.back();
	}
}

bool Game::Initialize()
{
	mRenderer = new Renderer(this);
	mRenderer->Initialize(1024, 768);

	LoadData();
	mTicksCount = SDL_GetTicks();
	return true;
}

void Game::Update()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	UnloadData();
	mRenderer->Shutdown();
	delete mRenderer;
}
void Game::AddActor(Actor *actor)
{
	if (mUpdatingActors)
		mPendingActors.emplace_back(actor);
	else
		mActors.emplace_back(actor);
}

void Game::RemoveActor(Actor *actor)
{
	auto iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
		mActors.erase(iter);
}

void Game::ProcessInput()
{
	SDL_Event event;

	// returns true if there are events in the q
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		actor->ProcessInput(state);
	}
	mUpdatingActors = false;
}

static float sun = 0.f;

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		; // limits to 60 fps

	// calc delta time
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.f;
	mTicksCount = SDL_GetTicks();

	// clamp delta time to avoid big simulation jumps during debugging
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;

	for (auto pending : mPendingActors)
	{
		pending->CalculateWorldTransform();
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();

	std::vector<Actor *> actorsPendingRemoval;
	for (auto actor : mActors)
	{
		if (actor->GetState() == Actor::EPendingRemoval)
			actorsPendingRemoval.emplace_back(actor);
	}

	for (auto actor : actorsPendingRemoval)
	{
		delete actor;
	}
	/* DirectionalLight &dirLig = mRenderer->GetDirectionalLight(); */
	/* dirLig.mDirection = Vector3(0.f, Math::Cos(sun), Math::Sin(sun)); */
	sun += deltaTime;
}

void Game::GenerateOutput()
{
	mRenderer->Draw();
}
