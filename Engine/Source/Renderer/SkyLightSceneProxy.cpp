#include "SkyLightSceneProxy.h"

#include "Components/SkyLightComponent.h"

FSkyLightSceneProxy::FSkyLightSceneProxy(const USkyLightComponent& component)
{
	lightColor = component.GetLightColor() * component.GetIntensity();
}

FVector3 FSkyLightSceneProxy::GetLightColor() const
{
	return lightColor;
}

void FSkyLightSceneProxy::SetLightColor(FVector3 newLightColor)
{
	lightColor = newLightColor;
}
