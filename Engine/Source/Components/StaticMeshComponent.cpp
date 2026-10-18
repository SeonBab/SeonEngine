#include "StaticMeshComponent.h"

#include "Actor.h"
#include "Core/Logging/Log.h"
#include "Renderer/Scene.h"
#include "TransformComponent.h"
#include "World.h"

UStaticMeshComponent::UStaticMeshComponent(AActor& owner)
	: UActorComponent(owner)
{
}

UStaticMeshComponent::~UStaticMeshComponent()
{
	DestroyRenderState();
}

void UStaticMeshComponent::SetMeshData(const FStaticMeshData& newMeshData)
{
	meshData = newMeshData;
	MarkRenderStateDirty();
}

void UStaticMeshComponent::SetBaseColorImage(const FImage& newBaseColorImage)
{
	baseColorImage = newBaseColorImage;
	MarkRenderStateDirty();
}

const FStaticMeshData& UStaticMeshComponent::GetMeshData() const
{
	return meshData;
}

FMatrix4 UStaticMeshComponent::GetLocalToWorld() const
{
	const UTransformComponent* transform = GetOwner().GetComponent<UTransformComponent>();

	return transform->GetComponentTransform().ToMatrixWithScale();
}

bool UStaticMeshComponent::CreateRenderState(FRenderer& renderer)
{
	if (IsRenderStateCreated())
	{
		WriteLog("StaticMeshComponent", ELogLevel::Error, "Render state is already created.");
		return false;
	}

	if (!GetOwner().GetWorld().GetScene().AddStaticMesh(renderer, meshData, GetLocalToWorld(), sceneMeshId, baseColorImage ? &*baseColorImage : nullptr)) { return false; }

	bRenderStateDirty = false;

	return true;
}

bool UStaticMeshComponent::UpdateRenderState(FRenderer& renderer)
{
	if (!IsRenderStateCreated()) { return false; }
	if (!IsRenderStateDirty()) { return true; }
	if (!GetOwner().GetWorld().GetScene().SetMeshData(sceneMeshId, renderer, meshData, baseColorImage ? &*baseColorImage : nullptr)) { return false; }

	bRenderStateDirty = false;

	return true;
}

bool UStaticMeshComponent::SendRenderTransform()
{
	if (!IsRenderStateCreated()) { return false; }

	return GetOwner().GetWorld().GetScene().SetLocalToWorld(sceneMeshId, GetLocalToWorld());
}

void UStaticMeshComponent::DestroyRenderState()
{
	bRenderStateDirty = false;
	if (!IsRenderStateCreated()) { return; }

	(void)GetOwner().GetWorld().GetScene().RemoveStaticMesh(sceneMeshId);
	sceneMeshId = 0;
}

bool UStaticMeshComponent::IsRenderStateCreated() const
{
	return sceneMeshId != 0;
}

void UStaticMeshComponent::MarkRenderStateDirty()
{
	if (IsRenderStateCreated())
	{
		bRenderStateDirty = true;
	}
}

bool UStaticMeshComponent::IsRenderStateDirty() const
{
	return bRenderStateDirty;
}
