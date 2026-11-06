#include "TransformComponent.h"

Vector3 TransformComponent::GetPosition() const
{
    return position;
}

void TransformComponent::SetPosition(const Vector3& newPosition)
{
    position = newPosition;
}
