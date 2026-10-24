#include "StaticMeshSceneProxy.h"

#include "Renderer/Material.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/StaticMeshSceneProxyDesc.h"

#pragma region Lifecycle

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const FStaticMeshSceneProxyDesc& desc)
	: renderData(desc.StaticMesh ? desc.StaticMesh->GetRenderData() : nullptr),
	  material(desc.Material ? desc.Material : UMaterial::GetDefaultMaterial()),
	  localToWorld(desc.LocalToWorld)
{
}

#pragma endregion

#pragma region Resources

const FStaticMeshRenderData* FStaticMeshSceneProxy::GetRenderData() const
{
	return renderData;
}

FMaterialRenderProxy* FStaticMeshSceneProxy::GetMaterialRenderProxy() const
{
	return material->GetRenderProxy();
}

#pragma endregion

#pragma region Transform

FMatrix4 FStaticMeshSceneProxy::GetLocalToWorld() const
{
	return localToWorld;
}

void FStaticMeshSceneProxy::SetLocalToWorld(const FMatrix4& matrix)
{
	localToWorld = matrix;
}

#pragma endregion
