#include "StaticMeshComponent.h"

#include "Actor.h"
#include "Renderer/MeshRenderData.h"
#include "TransformComponent.h"

FStaticMeshComponent::FStaticMeshComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

void FStaticMeshComponent::SetVertices(const TArray<FPositionColorVertex>& inVertices)
{
	vertices = inVertices;
}

const TArray<FPositionColorVertex>& FStaticMeshComponent::GetVertices() const
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
