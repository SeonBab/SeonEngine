#include "Engine/Engine.h"

#include "Engine/GameInstance.h"
#include "Engine/WorldContext.h"
#include "Logging/LogMacros.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogEngineLifecycle);
}

UEngine::UEngine()
	: UObject(nullptr)
{
}

UEngine::~UEngine()
{
	DestroyWorldContext(worldContext ? worldContext->World() : nullptr);
}

void UEngine::Init(IEngineLoop* inEngineLoop)
{
	if (!inEngineLoop)
	{
		SE_LOG(LogEngineLifecycle, Fatal, "UEngine::Init requires a valid engine loop");
	}

	engineLoop = inEngineLoop;
}

void UEngine::Start()
{
}

void UEngine::PreExit()
{
}

FWorldContext& UEngine::CreateNewWorldContext()
{
	if (worldContext)
	{
		SE_LOG(LogEngineLifecycle, Fatal, "UEngine supports only one world context");
	}

	worldContext = MakeUnique<FWorldContext>();
	return *worldContext;
}

void UEngine::DestroyWorldContext(FWorld* inWorld)
{
	if (!worldContext || worldContext->World() != inWorld) { return; }

	if (worldContext->owningGameInstance)
	{
		worldContext->owningGameInstance->DetachWorldContext();
	}
	if (FWorld* world = worldContext->World())
	{
		world->SetGameInstance(nullptr);
	}
	worldContext.reset();
}
