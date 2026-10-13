#include "StaticMeshComponent.h"

#include "Actor.h"
#include "Engine/StaticMesh.h"
#include "Renderer/MeshRenderData.h"
#include "TransformComponent.h"

#include <utility>

FStaticMeshComponent::FStaticMeshComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

bool FStaticMeshComponent::SetStaticMesh(TSharedPtr<UStaticMesh> inStaticMesh)
{
	if (staticMesh == inStaticMesh)
	{
		return false;
	}

	staticMesh = std::move(inStaticMesh);
	return true;
}

UStaticMesh* FStaticMeshComponent::GetStaticMesh() const
{
	return staticMesh.get();
}

FMeshRenderData FStaticMeshComponent::GetRenderData() const
{
	FMeshRenderData renderData;
	if (const UStaticMesh* mesh = GetStaticMesh())
	{
		renderData.vertices = mesh->GetVertices();
		renderData.primitiveType = mesh->GetPrimitiveType();
	}

	const FTransformComponent* transformComponent = GetOwner().GetComponent<FTransformComponent>();
	renderData.localToWorld = transformComponent->GetComponentTransform().ToMatrixWithScale();

	return renderData;
}
