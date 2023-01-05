#pragma once
#include <cstdint>

class State
{
	public:
	State(const char *name, class StateMachine *owner, class AnimSpriteComponent *animSpriteComp);
	inline virtual void Update(float deltaTime) {}
	inline virtual void ProcessInput(const uint8_t *keyState) {}
	virtual void OnEnter();
	virtual void OnExit();
	inline const char *GetName() const { return mName; }

	protected:
	class StateMachine *mOwner;
	class AnimSpriteComponent *mAnimSpriteComponent;
	const char *mName;
};
