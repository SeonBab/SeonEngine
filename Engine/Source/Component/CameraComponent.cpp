#include "CameraComponent.h"

#include "CameraViewInfo.h"
#include "Component/TransformComponent.h"
#include "Core/Transform.h"
#include "GameObject.h"

bool CameraComponent::SetVerticalFovDegrees(float newVerticalFovDegrees)
{
	if (newVerticalFovDegrees <= 0.0f || newVerticalFovDegrees >= 180.0f) { return false; }

	verticalFovDegrees = newVerticalFovDegrees;
	return true;
}

bool CameraComponent::SetClipDistances(float newNearClipDistance, float newFarClipDistance)
{
	if (newNearClipDistance <= 0.0f || newFarClipDistance <= newNearClipDistance) { return false; }

	nearClipDistance = newNearClipDistance;
	farClipDistance = newFarClipDistance;
	return true;
}

bool CameraComponent::GetCameraViewInfo(CameraViewInfo& outCameraViewInfo) const
{
	const GameObject* owner = GetGameObject();
	if (owner == nullptr) { return false; }

	const TransformComponent* transformComponent = owner->GetComponent<TransformComponent>();
	if (transformComponent == nullptr) { return false; }

	const Transform transform = transformComponent->GetTransform();
	outCameraViewInfo.position = transform.position;
	outCameraViewInfo.rotation = transform.rotation;
	outCameraViewInfo.verticalFovDegrees = verticalFovDegrees;
	outCameraViewInfo.nearClipDistance = nearClipDistance;
	outCameraViewInfo.farClipDistance = farClipDistance;
	return true;
}
