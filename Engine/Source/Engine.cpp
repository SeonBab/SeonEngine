#include "Engine.h"

#include "AssetManager.h"
#include "Core/Logging/Log.h"
#include "Core/Misc/Paths.h"
#include "Maps/ActorTypeRegistry.h"
#include "Maps/ComponentTypeRegistry.h"
#include "Maps/GameTypeRegistration.h"
#include "Maps/MapLoader.h"
#include "World.h"

#include <cassert>
#include <utility>

#pragma region Lifecycle

UEngine::UEngine() = default;

UEngine::~UEngine()
{
	// 파생 부분이 이미 정리된 객체는 잔여 World의 종료 코드에서도 전역으로 조회하지 않는다.
	if (activeEngine == this)
	{
		activeEngine = nullptr;
	}
	world.reset();
	assetManager.reset();
}

#pragma endregion

#pragma region Services

UEngine* UEngine::activeEngine = nullptr;

UEngine* UEngine::Get()
{
	return activeEngine;
}

UAssetManager& UEngine::GetAssetManager() noexcept
{
	assert(assetManager);

	return *assetManager;
}

#pragma endregion

#pragma region Execution

bool UEngine::Initialize(IRenderer& renderer)
{
	if (world) { return false; }
	if (activeEngine != nullptr && activeEngine != this)
	{
		WriteLog("Engine", ELogLevel::Error, "Initializing another engine requires the active engine to finish PreExit first.");
		return false;
	}

	TUniquePtr<UAssetManager> newAssetManager = MakeUnique<UAssetManager>(renderer);
	TUniquePtr<UWorld> newWorld = MakeUnique<UWorld>();
	assetManager = std::move(newAssetManager);
	world = std::move(newWorld);
	activeEngine = this;

	return true;
}

bool UEngine::LoadMap(FStringView contentPath)
{
	if (activeEngine != this || !world || world->HasBegunPlay()) { return false; }

	const auto path = FPaths::ResolveContentPath(contentPath);
	if (!path) { return false; }

	FActorTypeRegistry actorTypes;
	FComponentTypeRegistry componentTypes;
	if (!actorTypes.RegisterBuiltInTypes() || !componentTypes.RegisterBuiltInTypes()) { return false; }
	if (!RegisterGameTypes(actorTypes, componentTypes)) { return false; }

	return FMapLoader::LoadMapFile(*path, actorTypes, componentTypes, *world);
}

void UEngine::Start()
{
}

void UEngine::PreExit()
{
	world.reset();
	if (activeEngine == this)
	{
		activeEngine = nullptr;
	}
	assetManager.reset();
}

#pragma endregion
