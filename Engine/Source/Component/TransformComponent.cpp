#include "TransformComponent.h"

#include "Core/Quaternion.h"

Transform TransformComponent::GetTransform() const
{
	return transform;
}

Vector3 TransformComponent::GetPosition() const
{
	return transform.position;
}

void TransformComponent::SetPosition(const Vector3& newPosition)
{
	transform.position = newPosition;
}

EulerAngles TransformComponent::GetRotation() const
{
	return transform.rotation.ToEulerAngles();
}

void TransformComponent::SetRotation(const EulerAngles& newRotation)
{
	transform.rotation = Quaternion::CreateFromEulerAngles(newRotation);
}

Vector3 TransformComponent::GetScale() const
{
	return transform.scale;
}

void TransformComponent::SetScale(const Vector3& newScale)
{
	transform.scale = newScale;
}
