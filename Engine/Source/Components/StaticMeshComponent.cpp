#include "StaticMeshComponent.h"

#include "Actor.h"
#include "Renderer/Scene.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/StaticMeshSceneProxy.h"
#include "Renderer/StaticMeshSceneProxyDesc.h"
#include "TransformComponent.h"
#include "World.h"

#include <utility>

#pragma region Lifecycle

UStaticMeshComponent::UStaticMeshComponent(AActor& owner)
	: URenderStateComponent(owner)
{
}

UStaticMeshComponent::~UStaticMeshComponent()
{
	DestroyRenderState();
}

#pragma endregion

#pragma region Mesh

bool UStaticMeshComponent::SetStaticMesh(TSharedPtr<UStaticMesh> newStaticMesh)
{
	if (staticMesh == newStaticMesh) { return false; }

	staticMesh = std::move(newStaticMesh);
	MarkRenderStateDirty();

	return true;
}

UStaticMesh* UStaticMeshComponent::GetStaticMesh() const
{
	return staticMesh.get();
}

#pragma endregion

#pragma region Material

void UStaticMeshComponent::SetMaterial(int32 materialIndex, TSharedPtr<UMaterialInterface> newMaterial)
{
	if (materialIndex < 0) { return; }

	const TArray<TSharedPtr<UMaterialInterface>>::size_type index = static_cast<TArray<TSharedPtr<UMaterialInterface>>::size_type>(materialIndex);
	if (index < overrideMaterials.size() && overrideMaterials[index] == newMaterial) { return; }
	if (index >= overrideMaterials.size())
	{
		if (!newMaterial) { return; }

		overrideMaterials.resize(index + 1);
	}
	overrideMaterials[index] = std::move(newMaterial);
	MarkRenderStateDirty();
}

UMaterialInterface* UStaticMeshComponent::GetMaterial(int32 materialIndex) const
{
	return ResolveMaterial(materialIndex).get();
}

TSharedPtr<UMaterialInterface> UStaticMeshComponent::ResolveMaterial(int32 materialIndex) const
{
	if (materialIndex < 0) { return nullptr; }

	const TArray<TSharedPtr<UMaterialInterface>>::size_type index = static_cast<TArray<TSharedPtr<UMaterialInterface>>::size_type>(materialIndex);
	if (index < overrideMaterials.size() && overrideMaterials[index])
	{
		return overrideMaterials[index];
	}
	if (!staticMesh) { return nullptr; }

	return staticMesh->GetMaterial(materialIndex);
}

#pragma endregion

#pragma region RenderState

bool UStaticMeshComponent::CreateRenderState()
{
	if (bRenderStateCreated) { return false; }
	if (!ApplyRenderState()) { return false; }

	bRenderStateCreated = true;

	return true;
}

bool UStaticMeshComponent::SendRenderUpdates()
{
	if (!bRenderStateCreated) { return false; }

	if (bRenderStateDirty && !ApplyRenderState()) { return false; }
	if (!sceneProxy) { return true; }

	return GetOwner().GetWorld().GetScene().SetLocalToWorld(sceneProxy, GetLocalToWorld());
}

void UStaticMeshComponent::DestroyRenderState()
{
	if (sceneProxy)
	{
		(void)GetOwner().GetWorld().GetScene().RemoveStaticMesh(sceneProxy);
		sceneProxy = nullptr;
	}
	renderStateMaterial.reset();
	renderStateStaticMesh.reset();
	bRenderStateCreated = false;
	bRenderStateDirty = false;
}

bool UStaticMeshComponent::IsRenderStateCreated() const
{
	return bRenderStateCreated;
}

void UStaticMeshComponent::MarkRenderStateDirty()
{
	if (bRenderStateCreated)
	{
		bRenderStateDirty = true;
	}
}

bool UStaticMeshComponent::IsRenderStateDirty() const
{
	return bRenderStateDirty;
}

FMatrix4 UStaticMeshComponent::GetLocalToWorld() const
{
	const UTransformComponent* transform = GetOwner().GetComponent<UTransformComponent>();

	return transform->GetComponentTransform().ToMatrixWithScale();
}

bool UStaticMeshComponent::ApplyRenderState()
{
	// 후보 생성부터 교체 완료까지 새 자원을 유지한다. 기존 참조는 Scene의 이전 Proxy 제거 뒤에 놓는다.
	TSharedPtr<UStaticMesh> newStaticMesh = staticMesh;
	// 현재 형상은 섹션 구분 없는 단일 그리기이므로 재질 슬롯 0을 사용한다.
	TSharedPtr<UMaterialInterface> newMaterial = ResolveMaterial(0);
	TUniquePtr<FStaticMeshSceneProxy> newProxy;
	if (newStaticMesh)
	{
		const FStaticMeshSceneProxyDesc desc(*this);
		newProxy = MakeUnique<FStaticMeshSceneProxy>(desc);
	}

	FScene& scene = GetOwner().GetWorld().GetScene();
	FStaticMeshSceneProxy* newProxyAddress = newProxy.get();
	if (sceneProxy)
	{
		if (newProxy)
		{
			if (!scene.ReplaceStaticMesh(sceneProxy, newProxy)) { return false; }
		}
		else if (!scene.RemoveStaticMesh(sceneProxy))
		{
			return false;
		}
	}
	else if (newProxy && !scene.AddStaticMesh(newProxy))
	{
		return false;
	}

	sceneProxy = newProxyAddress;
	renderStateStaticMesh = std::move(newStaticMesh);
	renderStateMaterial = std::move(newMaterial);
	bRenderStateDirty = false;

	return true;
}

#pragma endregion
