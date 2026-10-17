#include "Scene.h"

#include "Logging/Log.h"
#include "Renderer/Renderer.h"

#include <algorithm>
#include <utility>

bool FScene::AddStaticMesh(FRenderer& renderer, const FStaticMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId)
{
	if (nextMeshId == 0)
	{
		WriteLog("Scene", ELogLevel::Error, "Scene mesh IDs are exhausted.");
		return false;
	}

	FStaticMeshEntry entry;
	if (!renderer.CreateMesh(meshData, entry.renderData))
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

bool FScene::SetMeshData(uint64 meshId, FRenderer& renderer, const FStaticMeshData& meshData)
{
	FStaticMeshEntry* entry = FindMeshEntry(meshId);
	if (entry == nullptr) { return false; }

	return renderer.CreateMesh(meshData, entry->renderData);
}

bool FScene::AddStaticMesh(FRenderer& renderer, const FModelMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId)
{
	if (nextMeshId == 0)
	{
		WriteLog("Scene", ELogLevel::Error, "Scene mesh IDs are exhausted.");
		return false;
	}

	FStaticMeshEntry entry;
	if (!renderer.CreateMesh(meshData, entry.renderData))
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

bool FScene::SetMeshData(uint64 meshId, FRenderer& renderer, const FModelMeshData& meshData)
{
	FStaticMeshEntry* entry = FindMeshEntry(meshId);
	if (entry == nullptr) { return false; }

	return renderer.CreateMesh(meshData, entry->renderData);
}

bool FScene::AddStaticMesh(FRenderer& renderer, const FModelMeshData& meshData, const FImage& baseColorImage, const FMatrix4& localToWorld, uint64& outMeshId)
{
	if (nextMeshId == 0)
	{
		WriteLog("Scene", ELogLevel::Error, "Scene mesh IDs are exhausted.");
		return false;
	}

	FStaticMeshEntry entry;
	if (!renderer.CreateMesh(meshData, baseColorImage, entry.renderData))
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

bool FScene::SetMeshData(uint64 meshId, FRenderer& renderer, const FModelMeshData& meshData, const FImage& baseColorImage)
{
	FStaticMeshEntry* entry = FindMeshEntry(meshId);
	if (entry == nullptr) { return false; }

	return renderer.CreateMesh(meshData, baseColorImage, entry->renderData);
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

bool FScene::RenderFrame(FRenderer& renderer, int width, int height)
{
	if (!renderer.BeginFrame(width, height))
	{
		return false;
	}
	for (const FStaticMeshEntry& entry : staticMeshes)
	{
		if (!renderer.SetLocalToWorld(entry.localToWorld) || !renderer.DrawMesh(entry.renderData))
		{
			return false;
		}
	}

	return renderer.EndFrame();
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
