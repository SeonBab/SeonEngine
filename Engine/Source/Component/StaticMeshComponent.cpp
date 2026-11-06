#include "StaticMeshComponent.h"

#include "GameObject.h"
#include "Renderer/RenderScene.h"
#include "World.h"

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

void StaticMeshComponent::SetStaticMesh(std::shared_ptr<const StaticMeshData> newStaticMesh)
{
	staticMesh = std::move(newStaticMesh);
}
