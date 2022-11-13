#include "Actor.h"
#include "Component.h"
#include <algorithm>

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

