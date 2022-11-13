#pragma once
#include "Structs.h"
#include <vector>

class Actor
{
public:
	enum State
	{
		EActive,
		EPaused,
		EPendingRemoval
	};

	Actor(class Game* game) : mState(EActive), mPosition(0, 0), mScale(0), mRotation(0), mGame(game) {}
	virtual ~Actor() = default;

	void Update(float deltaTime); // To be called from Game
	void UpdateComponents(float deltaTime);
	virtual void UpdateActor(float deltaTime) {}

	void AddComponent(class Component* component);
	void RemoveComponent(class Component* component);
	State GetState() const { return mState; }
	void SetState(State state) { mState = state; }
private:
	State mState;
	Vector2 mPosition;
	float mScale;
	float mRotation;
	std::vector<class Component*> mComponents;
	class Game* mGame;

};

