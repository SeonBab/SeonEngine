#include "Engine/GameEngine.h"

#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogGameEngine);
}

UGameEngine::UGameEngine() = default;
UGameEngine::~UGameEngine() = default;

void UGameEngine::Init(IEngineLoop* inEngineLoop)
{
	if (gameInstance)
	{
		SE_LOG(LogGameEngine, Fatal, "UGameEngine::Init requires no existing game instance");
	}

	UEngine::Init(inEngineLoop);
	gameInstance = MakeUnique<UGameInstance>(*this);
	gameInstance->InitializeStandalone();
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
