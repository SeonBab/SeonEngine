#include "DirectionalLightComponent.h"

#include "Actor.h"
#include "Renderer/DirectionalLightSceneProxy.h"
#include "Renderer/Scene.h"
#include "TransformComponent.h"
#include "World.h"

#include <utility>

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

	auto newProxy = MakeUnique<FDirectionalLightSceneProxy>(*this);
	if (!GetOwner().GetWorld().GetScene().AddDirectionalLight(*newProxy)) { return false; }

	// 소유 포인터를 이동해도 Scene이 빌린 Proxy 객체의 주소는 유지된다.
	sceneProxy = std::move(newProxy);

	return true;
}

bool UDirectionalLightComponent::UpdateRenderState()
{
	if (!IsRenderStateCreated()) { return false; }

	if (GetOwner().GetWorld().GetScene().GetDirectionalLight() != sceneProxy.get()) { return false; }

	sceneProxy->SetLightDirection(GetLightDirection());
	sceneProxy->SetLightColor(GetLightColor() * GetIntensity());

	return true;
}

void UDirectionalLightComponent::DestroyRenderState()
{
	if (!IsRenderStateCreated()) { return; }

	// Scene이 빌린 주소를 먼저 제거한 뒤 Proxy를 파괴한다.
	(void)GetOwner().GetWorld().GetScene().RemoveDirectionalLight(*sceneProxy);
	sceneProxy.reset();
}

bool UDirectionalLightComponent::IsRenderStateCreated() const
{
	return sceneProxy != nullptr;
}
