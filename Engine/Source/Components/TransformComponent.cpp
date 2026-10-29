#include "TransformComponent.h"

#include "Actor.h"

#pragma region Lifecycle

UTransformComponent::UTransformComponent(AActor& owner)
	: UActorComponent(owner)
{
}

void UTransformComponent::PostLoad()
{
	UActorComponent::PostLoad();
	UpdateComponentToWorld();
}

#pragma endregion

#pragma region RelativeTransform

FTransform UTransformComponent::GetTransform() const
{
	return {relativePosition, relativeRotation.ToQuat(), relativeScale};
}

void UTransformComponent::SetTransform(const FTransform& newTransform)
{
	relativePosition = newTransform.position;
	relativeRotation = newTransform.rotation.ToRotator();
	relativeScale = newTransform.scale;
	UpdateComponentToWorld();
}

FVector3 UTransformComponent::GetPosition() const
{
	return relativePosition;
}

void UTransformComponent::SetPosition(FVector3 newPosition)
{
	relativePosition = newPosition;
	UpdateComponentToWorld();
}

FQuat UTransformComponent::GetQuaternion() const
{
	return relativeRotation.ToQuat();
}

void UTransformComponent::SetQuaternion(FQuat newRotation)
{
	relativeRotation = newRotation.ToRotator();
	UpdateComponentToWorld();
}

FRotator UTransformComponent::GetRotation() const
{
	return relativeRotation;
}

void UTransformComponent::SetRotation(FRotator newRotation)
{
	relativeRotation = newRotation;
	UpdateComponentToWorld();
}

FVector3 UTransformComponent::GetScale() const
{
	return relativeScale;
}

void UTransformComponent::SetScale(FVector3 newScale)
{
	relativeScale = newScale;
	UpdateComponentToWorld();
}

#pragma endregion

#pragma region WorldTransform

FTransform UTransformComponent::GetComponentTransform() const
{
	return componentToWorld;
}

void UTransformComponent::UpdateComponentToWorld()
{
	componentToWorld = GetTransform();
	const AActor* parent = GetOwner().GetParent();
	if (parent)
	{
		const UTransformComponent* parentTransform = parent->GetComponent<UTransformComponent>();
		componentToWorld = componentToWorld * parentTransform->GetComponentTransform();
	}

	// 부모의 값을 먼저 확정한 뒤 자식에 전파한다. 이 경로는 콜백이나 계층 변경을 수행하지 않는다.
	const int32 childCount = GetOwner().GetChildCount();
	for (int32 index = 0; index < childCount; ++index)
	{
		UTransformComponent* childTransform = GetOwner().GetChild(index)->GetComponent<UTransformComponent>();
		childTransform->UpdateComponentToWorld();
	}
}

FVector3 UTransformComponent::GetForwardVector() const
{
	// 행 벡터 규칙에서 세 번째 행은 월드 회전으로 변환한 로컬 +Z축이다.
	const FMatrix4 matrix = GetComponentTransform().ToMatrixNoScale();

	return {matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]};
}

void UTransformComponent::SetComponentTransform(const FTransform& newTransform)
{
	const AActor* parent = GetOwner().GetParent();
	if (!parent)
	{
		SetTransform(newTransform);
		return;
	}

	const UTransformComponent* parentTransform = parent->GetComponent<UTransformComponent>();
	SetTransform(newTransform.GetRelativeTransform(parentTransform->GetComponentTransform()));
}

#pragma endregion
