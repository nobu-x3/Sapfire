#pragma once
#include "engine/State.h"

class IdleState : public State
{
	public:
	IdleState(class StateMachine *sm, class AnimSpriteComponent *animComp);
	void OnEnter() override;
	void Update(float deltaTime) override;
	void OnExit() override;

	inline const char *GetName() const override { return "idle"; }

	private:
	class AnimSpriteComponent *mAnimSpriteComp;
};
