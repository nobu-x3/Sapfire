#include "Component.h"
Component::Component(Actor *owner, int updateOrder) : mOwner(owner), mUpdateOrder(updateOrder)
{
	static unsigned int GUID{0};
	mGUID = GUID;
	GUID++;
	mOwner->AddComponent(this);
}

Component::~Component()
{
	mOwner->RemoveComponent(this);
}
