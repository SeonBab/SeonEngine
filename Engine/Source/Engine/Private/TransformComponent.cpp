#include "TransformComponent.h"

#include "Actor.h"

FTransformComponent::FTransformComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

FTransform FTransformComponent::GetTransform() const
{
	return transform;
}

void FTransformComponent::SetRelativeTransform(const FTransform& newTransform)
{
	transform = newTransform;
}

FTransform FTransformComponent::GetComponentTransform() const
{
	const FActor* parent = GetOwner().GetParent();
	if (!parent)
	{
		return transform;
	}
	const FTransformComponent* parentTransform = parent->GetComponent<FTransformComponent>();

	return transform * parentTransform->GetComponentTransform();
}

FVector3 FTransformComponent::GetPosition() const
{
	return transform.position;
}

void FTransformComponent::SetPosition(FVector3 newPosition)
{
	transform.position = newPosition;
}

FRotator FTransformComponent::GetRotation() const
{
	return transform.rotation.ToRotator();
}

void FTransformComponent::SetRotation(FRotator newRotation)
{
	transform.rotation = newRotation.ToQuat();
}

void FTransformComponent::SetRotation(FQuat newRotation)
{
	transform.rotation = newRotation;
}

FQuat FTransformComponent::GetQuaternion() const
{
	return transform.rotation;
}

FVector3 FTransformComponent::GetScale() const
{
	return transform.scale;
}

void FTransformComponent::SetScale(FVector3 newScale)
{
	transform.scale = newScale;
}
