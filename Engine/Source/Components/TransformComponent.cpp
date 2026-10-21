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

FVector3 UTransformComponent::GetForwardVector() const
{
	// 행 벡터 규칙에서 세 번째 행은 월드 회전으로 변환한 로컬 +Z축이다.
	const FMatrix4 matrix = GetComponentTransform().ToMatrixNoScale();

	return {matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]};
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
