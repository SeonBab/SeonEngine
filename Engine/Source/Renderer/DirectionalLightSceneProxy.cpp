#include "DirectionalLightSceneProxy.h"

#include "Components/DirectionalLightComponent.h"

FDirectionalLightSceneProxy::FDirectionalLightSceneProxy(const UDirectionalLightComponent& component)
	: lightDirection(component.GetLightDirection()),
	  lightColor(component.GetLightColor() * component.GetIntensity())
{
}

FVector3 FDirectionalLightSceneProxy::GetLightDirection() const
{
	return lightDirection;
}

FVector3 FDirectionalLightSceneProxy::GetLightColor() const
{
	return lightColor;
}

void FDirectionalLightSceneProxy::SetLightDirection(FVector3 newLightDirection)
{
	lightDirection = newLightDirection;
}

void FDirectionalLightSceneProxy::SetLightColor(FVector3 newLightColor)
{
	lightColor = newLightColor;
}
