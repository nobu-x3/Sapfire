#pragma once
#include "Actor.h"
class Component
{
public:
	// Constructor. The lower the updateOrder, the earlier the component updates.
	Component(class Actor* owner, int updateOrder = 100) : mActor(owner), mUpdateOrder(updateOrder) {}
	virtual ~Component() = default;

	virtual void Update(float deltaTime) {}
	int GetUpdateOrder() const { return mUpdateOrder; }
protected:
	Actor* mActor;
	int mUpdateOrder;

};

