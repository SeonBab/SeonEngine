#include "StaticMeshComponent.h"

#include "Actor.h"
#include "Renderer/MeshRenderData.h"
#include "TransformComponent.h"

FStaticMeshComponent::FStaticMeshComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

void FStaticMeshComponent::SetVertices(const TArray<FVector3>& inVertices)
{
	vertices = inVertices;
}

const TArray<FVector3>& FStaticMeshComponent::GetVertices() const
{
	return vertices;
}

FMeshRenderData FStaticMeshComponent::GetRenderData() const
{
	const FTransformComponent* transformComponent = GetOwner().GetComponent<FTransformComponent>();
	FMeshRenderData renderData;
	renderData.vertices = GetVertices();
	renderData.localToWorld = transformComponent->GetComponentTransform().ToMatrixWithScale();

	return renderData;
}
