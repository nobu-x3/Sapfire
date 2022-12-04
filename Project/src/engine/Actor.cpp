#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include <algorithm>

Actor::Actor(Game *game) : mGame(game), mPosition(Vector2(0, 0)), mRotation(0), mScale(1), mState(EActive)
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
	UpdateComponents(deltaTime);
	UpdateActor(deltaTime);
}

void Actor::UpdateComponents(float deltaTime)
{
	for (auto& component : mComponents)
		component->Update(deltaTime);
}

void Actor::AddComponent(Component* component)
{
	mComponents.emplace_back(component);
	std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) { return a->GetUpdateOrder() < b->GetUpdateOrder(); });
}

void Actor::RemoveComponent(Component* component)
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

void Actor::ActorInput(const uint8_t *keyState)
{
}
