#include "TransformComponent.h"

FTransformComponent::FTransformComponent(AActor& owner)
	: FActorComponent(owner)
{
}

FTransform FTransformComponent::GetTransform() const
{
	return transform;
}

void FTransformComponent::SetTransform(const FTransform& newTransform)
{
	transform = newTransform;
}

FVector3 FTransformComponent::GetPosition() const
{
	return transform.position;
}

void FTransformComponent::SetPosition(FVector3 newPosition)
{
	transform.position = newPosition;
}

FQuat FTransformComponent::GetQuaternion() const
{
	return transform.rotation;
}

void FTransformComponent::SetQuaternion(FQuat newRotation)
{
	transform.rotation = newRotation;
}

FRotator FTransformComponent::GetRotation() const
{
	return GetQuaternion().ToRotator();
}

void FTransformComponent::SetRotation(FRotator newRotation)
{
	SetQuaternion(newRotation.ToQuat());
}

FVector3 FTransformComponent::GetScale() const
{
	return transform.scale;
}

void FTransformComponent::SetScale(FVector3 newScale)
{
	transform.scale = newScale;
}
