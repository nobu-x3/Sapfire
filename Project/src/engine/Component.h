#pragma once
#include "Actor.h"
class Component
{
public:
	// Constructor. The lower the updateOrder, the earlier the component updates.
  Component(class Actor *owner, int updateOrder = 100);
  virtual ~Component();

  virtual void Update(float deltaTime) {}
  inline virtual void OnUpdateWorldTransform() {}
  inline int GetUpdateOrder() const { return mUpdateOrder; }
  inline virtual void ProcessInput(const uint8_t *keyState) {}
  inline Actor *GetOwner() const { return mOwner; }
  inline virtual void OnWorldTransformUpdated() {}

  protected:
  Actor *mOwner;
  int mUpdateOrder;

};
