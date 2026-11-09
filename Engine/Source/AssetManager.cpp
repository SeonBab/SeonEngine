#include "AssetManager.h"

#include "Core/Log.h"
#include "GLTFLoader.h"
#include "Mesh/StaticMeshData.h"
#include "Paths.h"

#include <cassert>
#include <string>
#include <utility>

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager()
{
	assert(instance == nullptr && "AssetManager는 한 번에 하나만 존재해야 한다.");
	instance = this;
}

AssetManager::~AssetManager()
{
	if (instance == this) { instance = nullptr; }
}

AssetManager& AssetManager::Get()
{
	assert(instance != nullptr && "AssetManager가 없을 때 Get을 호출했다.");
	return *instance;
}

std::shared_ptr<const StaticMeshData> AssetManager::LoadStaticMesh(std::string_view relativePath)
{
	std::filesystem::path absolutePath;
	if (!Paths::GetAbsolutePath(relativePath, absolutePath))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to resolve static mesh path: " + std::string(relativePath));
		return nullptr;
	}

	absolutePath = absolutePath.lexically_normal();
	absolutePath.make_preferred();

	const auto cachedMesh = staticMeshes.find(absolutePath);
	if (cachedMesh != staticMeshes.end()) { return cachedMesh->second; }

	StaticMeshData meshData;
	if (!GLTFLoader::LoadStaticMesh(absolutePath, meshData))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to load static mesh: " + std::string(relativePath));
		return nullptr;
	}

	std::shared_ptr<const StaticMeshData> mesh = std::make_shared<const StaticMeshData>(std::move(meshData));
	staticMeshes.emplace(std::move(absolutePath), mesh);
	return mesh;
}
