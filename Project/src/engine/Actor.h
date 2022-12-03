#pragma once
#include "Math.h"
#include <vector>
class Game;
class Actor
{
  public:
    enum State
    {
	    EActive,
	    EPaused,
	    EPendingRemoval
    };

    Actor(Game *mGame);
    virtual ~Actor () = default;

    void Update (float deltaTime); // To be called from Game
    void UpdateComponents (float deltaTime);
    virtual void UpdateActor(float deltaTime) {}

    void AddComponent (class Component *component);
    void RemoveComponent (class Component *component);
    inline State GetState() const { return mState; }
    inline void SetState(State state) { mState = state; }
    inline Game *GetGame() const { return mGame; }
    inline Vector2 GetPosition() const { return mPosition; }
    inline void SetPosition(Vector2 pos) { mPosition = pos; }
    inline float GetRotation() const { return mRotation; }
    inline void SetRotation(float rotation) { mRotation = rotation; }
    inline void SetScale(float scale) { mScale = scale; }
    inline float GetScale() const { return mScale; }
    inline Vector2 GetForwardVector() const { return Vector2(Math::Cos(mRotation), -Math::Sin(mRotation)); }

  private:
    State mState;
    Vector2 mPosition;
    float mScale;
    float mRotation;
    std::vector<class Component *> mComponents;
    Game *mGame;
};
