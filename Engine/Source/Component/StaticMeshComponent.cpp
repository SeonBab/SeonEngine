#include "StaticMeshComponent.h"

#include "GameObject.h"
#include "Renderer/Material.h"
#include "Renderer/RenderScene.h"
#include "World.h"

#include <cmath>
#include <utility>

StaticMeshComponent::~StaticMeshComponent()
{
	if (registeredRenderScene != nullptr)
	{
		registeredRenderScene->RemoveStaticMeshComponent(this);
	}
}

void StaticMeshComponent::OnRegister()
{
	GameObject* owner = GetGameObject();
	if (owner == nullptr) { return; }

	RenderScene& renderScene = owner->GetWorld().GetRenderScene();
	renderScene.AddStaticMeshComponent(this);
	registeredRenderScene = &renderScene;
}

void StaticMeshComponent::SetStaticMesh(std::shared_ptr<const StaticMesh> newStaticMesh)
{
	staticMesh = std::move(newStaticMesh);
}

bool StaticMeshComponent::SetMaterial(std::shared_ptr<const Material> newMaterial)
{
	if (newMaterial != nullptr)
	{
		const auto isInUnitRange = [](float value)
		{
			return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
		};

		const Vector3& color = newMaterial->baseColor;
		if (!isInUnitRange(color.x) || !isInUnitRange(color.y) || !isInUnitRange(color.z) || !isInUnitRange(newMaterial->roughness) || !isInUnitRange(newMaterial->metallic))
		{
			material.reset();
			return false;
		}
	}

	material = std::move(newMaterial);
	return true;
}
