#include "Engine/GameInstance.h"

#include "Engine/Engine.h"
#include "Engine/WorldContext.h"
#include "Logging/LogMacros.h"
#include "World.h"

#include <utility>

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogGameInstance);
}

UGameInstance::UGameInstance(UEngine& inEngine)
	: UObject(&inEngine)
{
}

UGameInstance::~UGameInstance()
{
	DetachWorldContext();
}

UEngine* UGameInstance::GetEngine() const
{
	UEngine* engine = dynamic_cast<UEngine*>(GetOuter());
	if (!engine)
	{
		SE_LOG(LogGameInstance, Fatal, "UGameInstance requires an engine Outer");
	}

	return engine;
}

FWorld* UGameInstance::GetWorld() const
{
	return worldContext ? worldContext->World() : nullptr;
}

void UGameInstance::InitializeStandalone()
{
	if (bStandaloneInitialized || worldContext)
	{
		SE_LOG(LogGameInstance, Fatal, "UGameInstance::InitializeStandalone may be called only once");
	}

	worldContext = &GetEngine()->CreateNewWorldContext();
	worldContext->owningGameInstance = this;

	TUniquePtr<FWorld> world = MakeUnique<FWorld>();
	world->SetGameInstance(this);
	worldContext->currentWorld = std::move(world);

	// 재진입하는 Init에서도 두 번째 준비를 허용하지 않는다.
	bStandaloneInitialized = true;
	Init();
}

void UGameInstance::Init()
{
}

void UGameInstance::StartGameInstance()
{
}

void UGameInstance::Shutdown()
{
	DetachWorldContext();
}

void UGameInstance::DetachWorldContext()
{
	if (!worldContext) { return; }

	if (worldContext->owningGameInstance == this)
	{
		worldContext->owningGameInstance = nullptr;
	}
	FWorld* world = worldContext->World();
	if (world && world->GetGameInstance() == this)
	{
		world->SetGameInstance(nullptr);
	}
	worldContext = nullptr;
}
