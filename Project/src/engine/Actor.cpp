#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include <SDL_log.h>
#include <algorithm>

Actor::Actor(Game *game)
    : mGame(game), mPosition(Vector3::Zero), mRotation(Quaternion::Identity), mScale(1), mState(EActive),
      mRecalculateWorldTransform(true)
{
	game->AddActor(this);
}

Actor::~Actor()
{
	mGame->RemoveActor(this);
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
	for (auto& component : mComponents)
		component->Update(deltaTime);
}

void Actor::AddComponent(Component* component)
{
	mComponents.emplace_back(std::make_unique<Component>(std::move(*component)));
	std::sort(mComponents.begin(), mComponents.end(),
		  [](std::unique_ptr<Component> &a, std::unique_ptr<Component> &b) {
			  return a->GetUpdateOrder() < b->GetUpdateOrder();
		  });
}

void Actor::RemoveComponent(Component* component)
{
	auto iter = std::find_if(mComponents.begin(), mComponents.end(),
				 [&](std::unique_ptr<Component> &p) { return p->GetGUID() == component->GetGUID(); });
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void Actor::ProcessInput(const uint8_t *keyState)
{
	if (mState == EActive)
	{
		for (auto &comp : mComponents)
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
		for (auto &comp : mComponents)
		{
			comp->OnWorldTransformUpdated();
		}
	}
}

void Actor::ActorInput(const uint8_t *keyState)
{
}
