#pragma once
#include "Math.h"
class Actor
{
	public:
	enum ActorState
	{
		EActive,
		EPaused,
		EPendingRemoval
	};

	Actor(class Game *mGame);
	virtual ~Actor();

	void Update(float deltaTime); // To be called from Game
	void UpdateComponents(float deltaTime);
	virtual void UpdateActor(float deltaTime) {}

	void AddComponent(class Component *component);
	void RemoveComponent(class Component *component);
	void ProcessInput(const uint8_t *keyState);
	void ComputeWorldTransform();
	virtual void ActorInput(const uint8_t *keyState);
	inline ActorState GetState() const { return mState; }
	inline void SetState(ActorState state) { mState = state; }
	inline Game *GetGame() const { return mGame; }
	inline const Vector3 &GetPosition() const { return mPosition; }
	inline void SetPosition(Vector3 pos)
	{
		mPosition = pos;
		mRecalculateWorldTransform = true;
	}
	inline Quaternion GetRotation() const { return mRotation; }
	inline void SetRotation(Quaternion rotation)
	{
		mRotation = rotation;
		mRecalculateWorldTransform = true;
	}
	inline void SetScale(float scale)
	{
		mScale = scale;
		mRecalculateWorldTransform = true;
	}
	inline float GetScale() const { return mScale; }
	inline Vector3 GetForwardVector() const { return Vector3::Transform(Vector3::UnitX, mRotation); }
	void CalculateWorldTransform();
	inline const Matrix4 &GetWorldTransform() const { return mWorldTransform; }

	private:
	ActorState mState;
	Vector3 mPosition;
	float mScale;
	Quaternion mRotation;
	std::vector<class Component *> mComponents;
	Matrix4 mWorldTransform;
	class Game *mGame;
	bool mRecalculateWorldTransform;
};
