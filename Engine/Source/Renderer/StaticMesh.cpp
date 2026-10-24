#include "StaticMesh.h"

#include "Renderer/StaticMeshRenderData.h"

#include <utility>

#pragma region Lifecycle

UStaticMesh::UStaticMesh() = default;

UStaticMesh::~UStaticMesh() = default;

#pragma endregion

#pragma region RenderData

const FStaticMeshRenderData* UStaticMesh::GetRenderData() const
{
	return renderData.get();
}

void UStaticMesh::SetRenderData(TUniquePtr<FStaticMeshRenderData>&& newRenderData)
{
	renderData = std::move(newRenderData);
}

#pragma endregion

#pragma region Materials

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

#pragma endregion
