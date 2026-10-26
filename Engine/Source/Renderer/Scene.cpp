#include "Scene.h"

#include "Renderer/DirectionalLightSceneProxy.h"
#include "Renderer/MaterialRenderProxy.h"
#include "Renderer/Renderer.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SkyLightSceneProxy.h"
#include "Renderer/StaticMeshSceneProxy.h"

#include <algorithm>
#include <utility>

namespace
{
	bool IsMeshProxyReady(const FStaticMeshSceneProxy& proxy)
	{
		const FStaticMeshRenderData* data = proxy.GetRenderData();

		return data && proxy.GetMaterialRenderProxy();
	}
}

// unique_ptr 대상의 완전한 정의가 있는 cpp에서 생성/파괴한다.
#pragma region Lifecycle

FScene::FScene() = default;

FScene::~FScene() = default;

#pragma endregion

#pragma region StaticMeshes

bool FScene::AddStaticMesh(TUniquePtr<FStaticMeshSceneProxy>& proxy)
{
	if (!proxy || !IsMeshProxyReady(*proxy)) { return false; }

	staticMeshes.push_back(std::move(proxy));

	return true;
}

bool FScene::ReplaceStaticMesh(const FStaticMeshSceneProxy* oldProxy, TUniquePtr<FStaticMeshSceneProxy>& newProxy)
{
	if (!oldProxy || !newProxy || !IsMeshProxyReady(*newProxy)) { return false; }

	const auto entry = std::find_if(staticMeshes.begin(), staticMeshes.end(), [oldProxy](const TUniquePtr<FStaticMeshSceneProxy>& candidate)
	{
		return candidate.get() == oldProxy;
	});
	if (entry == staticMeshes.end()) { return false; }

	// 같은 위치에서 이전 Proxy를 파괴한다. 컴포넌트는 이 호출이 끝난 뒤 이전 자원 참조를 놓는다.
	*entry = std::move(newProxy);

	return true;
}

bool FScene::SetLocalToWorld(const FStaticMeshSceneProxy* proxy, const FMatrix4& matrix)
{
	if (!proxy) { return false; }

	for (const TUniquePtr<FStaticMeshSceneProxy>& entry : staticMeshes)
	{
		if (entry.get() == proxy)
		{
			entry->SetLocalToWorld(matrix);

			return true;
		}
	}

	return false;
}

bool FScene::RemoveStaticMesh(const FStaticMeshSceneProxy* proxy)
{
	if (!proxy) { return false; }

	const auto entry = std::find_if(staticMeshes.begin(), staticMeshes.end(), [proxy](const TUniquePtr<FStaticMeshSceneProxy>& candidate)
	{
		return candidate.get() == proxy;
	});
	if (entry == staticMeshes.end()) { return false; }

	staticMeshes.erase(entry);

	return true;
}

#pragma endregion

#pragma region DirectionalLight

bool FScene::AddDirectionalLight(const FDirectionalLightSceneProxy& proxy)
{
	if (directionalLight) { return false; }

	directionalLight = &proxy;

	return true;
}

bool FScene::RemoveDirectionalLight(const FDirectionalLightSceneProxy& proxy)
{
	if (directionalLight != &proxy) { return false; }

	directionalLight = nullptr;

	return true;
}

const FDirectionalLightSceneProxy* FScene::GetDirectionalLight() const
{
	return directionalLight;
}

#pragma endregion

#pragma region SkyLight

bool FScene::SetSkyLight(const FSkyLightSceneProxy& proxy)
{
	if (IsSkyLightRegistered(proxy)) { return false; }

	skyLightStack.push_back(&proxy);

	return true;
}

bool FScene::DisableSkyLight(const FSkyLightSceneProxy& proxy)
{
	const auto entry = std::find(skyLightStack.begin(), skyLightStack.end(), &proxy);
	if (entry == skyLightStack.end()) { return false; }

	skyLightStack.erase(entry);

	return true;
}

bool FScene::IsSkyLightRegistered(const FSkyLightSceneProxy& proxy) const
{
	return std::find(skyLightStack.begin(), skyLightStack.end(), &proxy) != skyLightStack.end();
}

const FSkyLightSceneProxy* FScene::GetSkyLight() const
{
	if (skyLightStack.empty()) { return nullptr; }

	return skyLightStack.back();
}

#pragma endregion

#pragma region Rendering

bool FScene::Draw(IRenderer& renderer, const FRenderView& view)
{
	FSceneRenderData data;
	data.view = view;
	// 미등록 프레임에도 유효한 기본 방향과 광량 0을 전달해 이전 광량을 지운다.
	if (directionalLight)
	{
		data.lightDirection = directionalLight->GetLightDirection();
		data.lightColor = directionalLight->GetLightColor();
	}

	// Scene 전환과 마지막 SkyLight 해제 뒤에도 이전 값이 남지 않도록 매번 전달한다.
	if (const FSkyLightSceneProxy* proxy = GetSkyLight())
	{
		data.skyLightColor = proxy->GetLightColor();
	}

	data.meshes.reserve(staticMeshes.size());
	for (const TUniquePtr<FStaticMeshSceneProxy>& proxy : staticMeshes)
	{
		const FStaticMeshRenderData* renderData = proxy->GetRenderData();
		const FMaterialRenderProxy* materialProxy = proxy->GetMaterialRenderProxy();
		if (!renderData || !materialProxy)
		{
			return false;
		}

		FMeshDrawItem item;
		item.mesh = renderData;
		item.localToWorld = proxy->GetLocalToWorld();
		item.baseColorTexture = materialProxy->GetBaseColorTexture();
		item.specularColor = materialProxy->GetSpecularColor();
		item.shininess = materialProxy->GetShininess();
		data.meshes.push_back(std::move(item));
	}

	return renderer.DrawScene(data);
}

#pragma endregion
