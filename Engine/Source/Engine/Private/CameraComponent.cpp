#include "CameraComponent.h"

#include "Actor.h"
#include "Camera/CameraTypes.h"
#include "TransformComponent.h"

FCameraComponent::FCameraComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

void FCameraComponent::SetFieldOfView(float inFieldOfView)
{
	fieldOfView = inFieldOfView;
}

float FCameraComponent::GetFieldOfView() const
{
	return fieldOfView;
}

void FCameraComponent::GetCameraView(FMinimalViewInfo& outView) const
{
	const FTransformComponent* transformComponent = GetOwner().GetComponent<FTransformComponent>();
	const FTransform worldTransform = transformComponent->GetComponentTransform();

	outView.location = worldTransform.position;
	outView.rotation = worldTransform.rotation.ToRotator();
	outView.fov = GetFieldOfView();
}
