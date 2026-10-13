#include "Engine/GameEngine.h"

#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"
#include "Slate/SceneViewport.h"
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
	// 기존 화면을 덮어쓰지 않는다. TODO: check 계열 도입 시 중복 준비 검사를 재검토한다.
	if (sceneViewport)
	{
		SE_LOG(LogGameEngine, Fatal, "Game engine initialization requires no existing scene viewport");
	}

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

	// 세션 준비 뒤 빈 키 상태의 화면을 생성한다. 입력 전달 연결은 별도로 준비한다.
	// TODO: 클라이언트·위젯 연결 도입 시 CreateGameViewport 분리와 생성 위임을 검토한다.
	sceneViewport = MakeShared<FSceneViewport>();
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

	// 세션 종료가 끝난 뒤 엔진의 화면 참조를 놓는다. 외부 공유 참조가 있으면 화면은 유지된다.
	sceneViewport.reset();

	UEngine::PreExit();
}

FWorld* UGameEngine::GetWorld() const
{
	return gameInstance ? gameInstance->GetWorld() : nullptr;
}
