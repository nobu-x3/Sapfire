#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include <SDL_log.h>

Actor::Actor(Game *game)
    : mGame(game), mPosition(Vector3::Zero), mRotation(Quaternion::Identity), mScale(1), mState(EActive),
      mRecalculateWorldTransform(true)
{
	game->AddActor(this);
}

Actor::~Actor()
{
	mGame->RemoveActor(this);
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}
void Actor::Update(float deltaTime)
{
	if (mState == ActorState::EActive)
	{
		CalculateWorldTransform();
		UpdateComponents(deltaTime);
		UpdateActor(deltaTime);
		CalculateWorldTransform(); // TODO: might be overkill
	}
}

void Actor::UpdateComponents(float deltaTime)
{
	for (auto &component : mComponents)
		component->Update(deltaTime);
}

void Actor::AddComponent(Component *component)
{
	mComponents.emplace_back(component);
	std::sort(mComponents.begin(), mComponents.end(),
		  [](Component *a, Component *b) { return a->GetUpdateOrder() < b->GetUpdateOrder(); });
}

void Actor::RemoveComponent(Component *component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void Actor::ProcessInput(const uint8_t *keyState)
{
	if (mState == EActive)
	{
		for (auto comp : mComponents)
		{
			comp->ProcessInput(keyState);
		}
		ActorInput(keyState);
	}
}

void Actor::CalculateWorldTransform()
{
	if (mRecalculateWorldTransform)
	{
		mRecalculateWorldTransform = false;
		mWorldTransform = Matrix4::CreateScale(mScale);
		mWorldTransform *= Matrix4::CreateFromQuaternion(mRotation);
		mWorldTransform *= Matrix4::CreateTranslation(mPosition);
		for (auto comp : mComponents)
		{
			comp->OnWorldTransformUpdated();
		}
	}
}

void Actor::ActorInput(const uint8_t *keyState)
{
}
