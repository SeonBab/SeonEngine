#include "DirectionalLightComponent.h"

#include "Component/TransformComponent.h"
#include "GameObject.h"
#include "Renderer/RenderScene.h"
#include "World.h"

#include <cmath>

DirectionalLightComponent::~DirectionalLightComponent()
{
	if (registeredRenderScene != nullptr)
	{
		registeredRenderScene->RemoveDirectionalLightComponent(this);
	}
}

void DirectionalLightComponent::OnRegister()
{
	GameObject* owner = GetGameObject();
	if (owner == nullptr) { return; }

	RenderScene& renderScene = owner->GetWorld().GetRenderScene();
	renderScene.AddDirectionalLightComponent(this);
	registeredRenderScene = &renderScene;
}

bool DirectionalLightComponent::GetLightDirection(Vector3& outDirection) const
{
	const GameObject* owner = GetGameObject();
	if (owner == nullptr) { return false; }

	const TransformComponent* transform = owner->GetComponent<TransformComponent>();
	if (transform == nullptr) { return false; }

	outDirection = transform->GetForwardVector();
	return true;
}

bool DirectionalLightComponent::SetColor(const Vector3& newColor)
{
	if (!std::isfinite(newColor.x) || !std::isfinite(newColor.y) || !std::isfinite(newColor.z)) { return false; }
	if (newColor.x < 0.0f || newColor.y < 0.0f || newColor.z < 0.0f) { return false; }

	color = newColor;
	return true;
}

bool DirectionalLightComponent::SetIntensity(float newIntensity)
{
	if (!std::isfinite(newIntensity) || newIntensity < 0.0f) { return false; }

	intensity = newIntensity;
	return true;
}

void DirectionalLightComponent::SetEnabled(bool enabled)
{
	isEnabled = enabled;
}
