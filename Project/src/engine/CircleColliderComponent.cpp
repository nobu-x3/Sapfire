#include "CircleColliderComponent.h"
bool CircleColliderComponent::Intersect(const CircleColliderComponent &a, const CircleColliderComponent &b)
{
	Vector2 diff = a.GetOwner()->GetPosition() - b.GetOwner()->GetPosition();
	float distSq = diff.LengthSq();

	float radiiSq = a.GetRadius() + b.GetRadius();
	radiiSq *= radiiSq;

	return distSq <= radiiSq;
}
