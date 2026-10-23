#include "CameraComponent.h"

#include "Actor.h"
#include "TransformComponent.h"

UCameraComponent::UCameraComponent(AActor& owner)
	: UActorComponent(owner)
{
}

ECameraProjectionMode UCameraComponent::GetProjectionMode() const
{
	return projectionMode;
}

void UCameraComponent::SetProjectionMode(ECameraProjectionMode newProjectionMode)
{
	projectionMode = newProjectionMode;
}

float UCameraComponent::GetHorizontalFovDegrees() const
{
	return horizontalFovDegrees;
}

void UCameraComponent::SetHorizontalFovDegrees(float newHorizontalFovDegrees)
{
	horizontalFovDegrees = newHorizontalFovDegrees;
}

float UCameraComponent::GetOrthographicWidth() const
{
	return orthographicWidth;
}

void UCameraComponent::SetOrthographicWidth(float newOrthographicWidth)
{
	orthographicWidth = newOrthographicWidth;
}

float UCameraComponent::GetNearClipDistance() const
{
	return nearClipDistance;
}

void UCameraComponent::SetNearClipDistance(float newNearClipDistance)
{
	nearClipDistance = newNearClipDistance;
}

float UCameraComponent::GetFarClipDistance() const
{
	return farClipDistance;
}

void UCameraComponent::SetFarClipDistance(float newFarClipDistance)
{
	farClipDistance = newFarClipDistance;
}

FTransform UCameraComponent::GetCameraWorldTransform() const
{
	const UTransformComponent* transform = GetOwner().GetComponent<UTransformComponent>();

	FTransform cameraTransform = transform->GetComponentTransform();
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};

	return cameraTransform;
}

FCameraViewInfo UCameraComponent::GetCameraView() const
{
	FCameraViewInfo view;
	const FTransform cameraTransform = GetCameraWorldTransform();
	view.location = cameraTransform.position;
	view.rotation = cameraTransform.rotation.ToRotator();
	view.projectionMode = projectionMode;
	view.horizontalFovDegrees = horizontalFovDegrees;
	view.orthographicWidth = orthographicWidth;
	view.nearClipDistance = nearClipDistance;
	view.farClipDistance = farClipDistance;

	return view;
}
