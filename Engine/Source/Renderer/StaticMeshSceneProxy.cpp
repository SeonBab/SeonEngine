#include "StaticMeshSceneProxy.h"

#include "Renderer/Material.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/StaticMeshSceneProxyDesc.h"

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const FStaticMeshSceneProxyDesc& desc)
	: renderData(desc.StaticMesh ? desc.StaticMesh->GetRenderData() : nullptr),
	  material(desc.Material ? desc.Material : UMaterial::GetDefaultMaterial()),
	  localToWorld(desc.LocalToWorld)
{
}

const FStaticMeshRenderData* FStaticMeshSceneProxy::GetRenderData() const
{
	return renderData;
}

FMaterialRenderProxy* FStaticMeshSceneProxy::GetMaterialRenderProxy() const
{
	return material->GetRenderProxy();
}

FMatrix4 FStaticMeshSceneProxy::GetLocalToWorld() const
{
	return localToWorld;
}

void FStaticMeshSceneProxy::SetLocalToWorld(const FMatrix4& matrix)
{
	localToWorld = matrix;
}
