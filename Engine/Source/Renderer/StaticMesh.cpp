#include "StaticMesh.h"

#include "Renderer/StaticMeshRenderData.h"

#include <utility>

UStaticMesh::UStaticMesh() = default;

UStaticMesh::~UStaticMesh() = default;

const FStaticMeshRenderData* UStaticMesh::GetRenderData() const
{
	return renderData.get();
}

void UStaticMesh::SetStaticMaterials(TArray<TSharedPtr<UMaterialInterface>> materials)
{
	staticMaterials = std::move(materials);
}

TSharedPtr<UMaterialInterface> UStaticMesh::GetMaterial(int32 materialIndex) const
{
	if (materialIndex < 0 || static_cast<TArray<TSharedPtr<UMaterialInterface>>::size_type>(materialIndex) >= staticMaterials.size())
	{
		return nullptr;
	}

	return staticMaterials[materialIndex];
}
