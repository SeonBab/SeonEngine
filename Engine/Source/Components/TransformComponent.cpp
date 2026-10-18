#include "TransformComponent.h"

#include "Actor.h"

UTransformComponent::UTransformComponent(AActor& owner)
	: UActorComponent(owner)
{
}

FTransform UTransformComponent::GetTransform() const
{
	return transform;
}

FTransform UTransformComponent::GetComponentTransform() const
{
	const AActor* parent = GetOwner().GetParent();
	if (!parent) { return transform; }

	const UTransformComponent* parentTransform = parent->GetComponent<UTransformComponent>();

	return transform * parentTransform->GetComponentTransform();
}

void UTransformComponent::SetTransform(const FTransform& newTransform)
{
	transform = newTransform;
}

void UTransformComponent::SetComponentTransform(const FTransform& newTransform)
{
	const AActor* parent = GetOwner().GetParent();
	if (!parent)
	{
		SetTransform(newTransform);
		return;
	}

	const auto* parentTransform = parent->GetComponent<UTransformComponent>();
	SetTransform(newTransform.GetRelativeTransform(parentTransform->GetComponentTransform()));
}

FVector3 UTransformComponent::GetPosition() const
{
	return transform.position;
}

void UTransformComponent::SetPosition(FVector3 newPosition)
{
	transform.position = newPosition;
}

FQuat UTransformComponent::GetQuaternion() const
{
	return transform.rotation;
}

void UTransformComponent::SetQuaternion(FQuat newRotation)
{
	transform.rotation = newRotation;
}

FRotator UTransformComponent::GetRotation() const
{
	return GetQuaternion().ToRotator();
}

void UTransformComponent::SetRotation(FRotator newRotation)
{
	SetQuaternion(newRotation.ToQuat());
}

FVector3 UTransformComponent::GetScale() const
{
	return transform.scale;
}

void UTransformComponent::SetScale(FVector3 newScale)
{
	transform.scale = newScale;
}
