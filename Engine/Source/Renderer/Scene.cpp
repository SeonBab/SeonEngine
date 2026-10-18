#include "Scene.h"

#include "Core/Logging/Log.h"
#include "Renderer/Renderer.h"

#include <algorithm>
#include <utility>

bool FScene::AddStaticMesh(FRenderer& renderer, const FStaticMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId, const FImage* baseColorImage)
{
	if (nextMeshId == 0)
	{
		WriteLog("Scene", ELogLevel::Error, "Scene mesh IDs are exhausted.");
		return false;
	}

	FStaticMeshEntry entry;
	if (!renderer.CreateMesh(meshData, entry.renderData, baseColorImage))
	{
		return false;
	}
	entry.meshId = nextMeshId;
	entry.localToWorld = localToWorld;
	staticMeshes.push_back(std::move(entry));
	outMeshId = nextMeshId;
	++nextMeshId;

	return true;
}

bool FScene::SetMeshData(uint64 meshId, FRenderer& renderer, const FStaticMeshData& meshData, const FImage* baseColorImage)
{
	FStaticMeshEntry* entry = FindMeshEntry(meshId);
	if (entry == nullptr) { return false; }

	return renderer.CreateMesh(meshData, entry->renderData, baseColorImage);
}

bool FScene::SetLocalToWorld(uint64 meshId, const FMatrix4& matrix)
{
	FStaticMeshEntry* entry = FindMeshEntry(meshId);
	if (entry == nullptr) { return false; }

	entry->localToWorld = matrix;

	return true;
}

bool FScene::RemoveStaticMesh(uint64 meshId)
{
	const TArray<FStaticMeshEntry>::iterator entry = std::find_if(staticMeshes.begin(), staticMeshes.end(), [meshId](const FStaticMeshEntry& candidate)
	{
		return candidate.meshId == meshId;
	});
	if (entry == staticMeshes.end()) { return false; }

	staticMeshes.erase(entry);

	return true;
}

bool FScene::Draw(FRenderer& renderer)
{
	for (const FStaticMeshEntry& entry : staticMeshes)
	{
		if (!renderer.SetLocalToWorld(entry.localToWorld) || !renderer.DrawMesh(entry.renderData))
		{
			return false;
		}
	}

	return true;
}

FScene::FStaticMeshEntry* FScene::FindMeshEntry(uint64 meshId)
{
	for (FStaticMeshEntry& entry : staticMeshes)
	{
		if (entry.meshId == meshId)
		{
			return &entry;
		}
	}

	return nullptr;
}
