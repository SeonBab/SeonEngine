#include "Engine/GameEngine.h"

#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogGameEngine);
}

FGameInstanceFactory UGameEngine::gameInstanceFactory = nullptr;

UGameEngine::UGameEngine() = default;
UGameEngine::~UGameEngine() = default;

bool UGameEngine::RegisterGameInstanceFactory(FGameInstanceFactory inFactory)
{
	if (!inFactory || gameInstanceFactory)
	{
		return false;
	}

	gameInstanceFactory = inFactory;

	return true;
}

bool UGameEngine::UnregisterGameInstanceFactory(FGameInstanceFactory inFactory)
{
	if (!inFactory || gameInstanceFactory != inFactory)
	{
		return false;
	}

	gameInstanceFactory = nullptr;

	return true;
}

void UGameEngine::Init(IEngineLoop* inEngineLoop)
{
	if (gameInstance)
	{
		SE_LOG(LogGameEngine, Fatal, "UGameEngine::Init requires no existing game instance");
	}

	UEngine::Init(inEngineLoop);
	if (gameInstanceFactory)
	{
		gameInstance = gameInstanceFactory(*this);
	}
	else
	{
		gameInstance = MakeUnique<UGameInstance>(*this);
	}

	if (!gameInstance)
	{
		SE_LOG(LogGameEngine, Fatal, "Game instance creation failed");
	}

	gameInstance->InitializeStandalone();
}

void UGameEngine::Start()
{
	if (!gameInstance)
	{
		SE_LOG(LogGameEngine, Fatal, "Starting the game requires an initialized game instance");
	}

	gameInstance->StartGameInstance();
}

void UGameEngine::Tick(float deltaSeconds)
{
	if (!gameInstance)
	{
		return;
	}

	FWorld* world = gameInstance->GetWorld();
	if (world)
	{
		world->Tick(deltaSeconds);
		world->PurgeDestroyedActors();
	}
}

void UGameEngine::PreExit()
{
	if (gameInstance)
	{
		FWorld* world = gameInstance->GetWorld();
		gameInstance->Shutdown();
		DestroyWorldContext(world);
		gameInstance.reset();
	}

	UEngine::PreExit();
}

FWorld* UGameEngine::GetWorld() const
{
	return gameInstance ? gameInstance->GetWorld() : nullptr;
}
