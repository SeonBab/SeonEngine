#include "DirectionalLightComponent.h"

#include "Actor.h"
#include "Renderer/DirectionalLightData.h"
#include "Renderer/Scene.h"
#include "TransformComponent.h"
#include "World.h"

UDirectionalLightComponent::UDirectionalLightComponent(AActor& owner)
	: UActorComponent(owner)
{
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
	DestroyRenderState();
}

FVector3 UDirectionalLightComponent::GetLightDirection() const
{
	const auto* transform = GetOwner().GetComponent<UTransformComponent>();

	return transform->GetForwardVector();
}

FVector3 UDirectionalLightComponent::GetLightColor() const
{
	return lightColor;
}

void UDirectionalLightComponent::SetLightColor(const FVector3& newLightColor)
{
	lightColor = newLightColor;
}

float UDirectionalLightComponent::GetIntensity() const
{
	return intensity;
}

void UDirectionalLightComponent::SetIntensity(float newIntensity)
{
	intensity = newIntensity;
}

bool UDirectionalLightComponent::CreateRenderState()
{
	if (IsRenderStateCreated()) { return false; }

	const FDirectionalLightData data{GetLightDirection(), GetLightColor(), GetIntensity()};

	return GetOwner().GetWorld().GetScene().AddDirectionalLight(data, sceneLightId);
}

bool UDirectionalLightComponent::UpdateRenderState()
{
	if (!IsRenderStateCreated()) { return false; }

	const FDirectionalLightData data{GetLightDirection(), GetLightColor(), GetIntensity()};

	return GetOwner().GetWorld().GetScene().SetDirectionalLight(sceneLightId, data);
}

void UDirectionalLightComponent::DestroyRenderState()
{
	if (!IsRenderStateCreated()) { return; }

	(void)GetOwner().GetWorld().GetScene().RemoveDirectionalLight(sceneLightId);
	sceneLightId = 0;
}

bool UDirectionalLightComponent::IsRenderStateCreated() const
{
	return sceneLightId != 0;
}
