#include "SkyLightComponent.h"

#include "Actor.h"
#include "Renderer/Scene.h"
#include "Renderer/SkyLightSceneProxy.h"
#include "World.h"

#include <utility>

#pragma region Lifecycle

USkyLightComponent::USkyLightComponent(AActor& owner)
	: URenderStateComponent(owner)
{
}

USkyLightComponent::~USkyLightComponent()
{
	DestroyRenderState();
}

#pragma endregion

#pragma region Light

FVector3 USkyLightComponent::GetLightColor() const
{
	return lightColor;
}

void USkyLightComponent::SetLightColor(const FVector3& newLightColor)
{
	lightColor = newLightColor;
}

float USkyLightComponent::GetIntensity() const
{
	return intensity;
}

void USkyLightComponent::SetIntensity(float newIntensity)
{
	intensity = newIntensity;
}

#pragma endregion

#pragma region RenderState

bool USkyLightComponent::CreateRenderState()
{
	if (IsRenderStateCreated()) { return false; }

	TUniquePtr<FSkyLightSceneProxy> newProxy = MakeUnique<FSkyLightSceneProxy>(*this);
	if (!GetOwner().GetWorld().GetScene().SetSkyLight(*newProxy)) { return false; }

	// 등록 성공 뒤 소유권만 옮긴다. Scene에 등록한 객체 주소는 유지된다.
	sceneProxy = std::move(newProxy);

	return true;
}

bool USkyLightComponent::SendRenderUpdates()
{
	if (!IsRenderStateCreated()) { return false; }

	const FScene& scene = GetOwner().GetWorld().GetScene();
	if (!scene.IsSkyLightRegistered(*sceneProxy)) { return false; }

	// 비활성 항목도 갱신해 이후 다시 활성화될 때 최신 설정을 사용한다.
	sceneProxy->SetLightColor(GetLightColor() * GetIntensity());

	return true;
}

void USkyLightComponent::DestroyRenderState()
{
	if (!IsRenderStateCreated()) { return; }

	(void)GetOwner().GetWorld().GetScene().DisableSkyLight(*sceneProxy);
	sceneProxy.reset();
}

bool USkyLightComponent::IsRenderStateCreated() const
{
	return sceneProxy != nullptr;
}

#pragma endregion
