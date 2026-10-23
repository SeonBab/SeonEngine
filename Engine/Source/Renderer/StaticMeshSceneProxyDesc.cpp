#include "StaticMeshSceneProxyDesc.h"

#include "Components/StaticMeshComponent.h"

FStaticMeshSceneProxyDesc::FStaticMeshSceneProxyDesc()
{
	LocalToWorld.SetIdentity();
}

FStaticMeshSceneProxyDesc::FStaticMeshSceneProxyDesc(const UStaticMeshComponent& component)
{
	StaticMesh = component.GetStaticMesh();
	Material = component.GetMaterial(0);
	LocalToWorld = component.GetLocalToWorld();
}
