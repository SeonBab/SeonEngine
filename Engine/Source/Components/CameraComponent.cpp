#include "CameraComponent.h"

#include "Actor.h"
#include "TransformComponent.h"

#include <cmath>
#include <numbers>

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

FMatrix4 UCameraComponent::GetViewMatrix() const
{
	return GetCameraWorldTransform().ToMatrixNoScale().Inverse();
}

FMatrix4 UCameraComponent::GetProjectionMatrix(float aspectRatio) const
{
	if (projectionMode == ECameraProjectionMode::Orthographic)
	{
		const float height = orthographicWidth / aspectRatio;
		const float depthScale = 1.0f / (farClipDistance - nearClipDistance);

		FMatrix4 projection;
		projection.m[0][0] = 2.0f / orthographicWidth;
		projection.m[1][1] = 2.0f / height;
		projection.m[2][2] = depthScale;
		projection.m[3][2] = -nearClipDistance * depthScale;
		projection.m[3][3] = 1.0f;

		return projection;
	}

	const float halfFovRadians = horizontalFovDegrees * std::numbers::pi_v<float> / 360.0f;
	const float xScale = 1.0f / std::tan(halfFovRadians);
	const float depthScale = farClipDistance / (farClipDistance - nearClipDistance);

	FMatrix4 projection;
	projection.m[0][0] = xScale;
	projection.m[1][1] = xScale * aspectRatio;
	projection.m[2][2] = depthScale;
	projection.m[2][3] = 1.0f;
	projection.m[3][2] = -nearClipDistance * depthScale;

	return projection;
}

FMatrix4 UCameraComponent::GetWorldToClipMatrix(float aspectRatio) const
{
	return GetViewMatrix() * GetProjectionMatrix(aspectRatio);
}
