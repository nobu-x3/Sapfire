#pragma once
#include "Math.h"
#include <cstdint>
#include <vector>
class Game;
class Actor
{
  public:
  enum ActorState
  {
	  EActive,
	  EPaused,
	  EPendingRemoval
  };

  Actor(Game *mGame);
  virtual ~Actor();

  void Update(float deltaTime); // To be called from Game
  void UpdateComponents(float deltaTime);
  virtual void UpdateActor(float deltaTime) {}

  void AddComponent(class Component *component);
  void RemoveComponent(class Component *component);
  void ProcessInput(const uint8_t *keyState);
  virtual void ActorInput(const uint8_t *keyState);
  inline ActorState GetState() const { return mState; }
  inline void SetState(ActorState state) { mState = state; }
  inline Game *GetGame() const { return mGame; }
  inline const Vector2 &GetPosition() const { return mPosition; }
  inline void SetPosition(Vector2 pos)
  {
	  mPosition = pos;
	  mRecalculateWorldTransform = true;
  }
  inline float GetRotation() const { return mRotation; }
  inline void SetRotation(float rotation)
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
  inline Vector2 GetForwardVector() const { return Vector2(Math::Cos(mRotation), Math::Sin(mRotation)); }
  void CalculateWorldTransform();
  inline const Matrix4 &GetWorldTransform() const { return mWorldTransform; }

  private:
  ActorState mState;
  Vector2 mPosition;
  float mScale;
  float mRotation;
  std::vector<class Component *> mComponents;
  Game *mGame;
  bool mRecalculateWorldTransform;
  Matrix4 mWorldTransform;
};
