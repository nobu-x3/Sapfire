#pragma once
#include "Actor.h"
class Component
{
public:
	// Constructor. The lower the updateOrder, the earlier the component updates.
  Component(class Actor *owner, int updateOrder = 100);
  virtual ~Component();

  virtual void Update(float deltaTime) {}
  inline int GetUpdateOrder() const { return mUpdateOrder; }

protected:
  Actor *mOwner;
  int mUpdateOrder;

};

