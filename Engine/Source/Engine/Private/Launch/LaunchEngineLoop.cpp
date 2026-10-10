#include "Launch/LaunchEngineLoop.h"

#include "Engine/GameEngine.h"
#include "GameModule.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogEngineLoop);
}

FEngineLoop::FEngineLoop() = default;
FEngineLoop::~FEngineLoop() = default;

int32 FEngineLoop::Init()
{
	if (engine)
	{
		SE_LOG(LogEngineLoop, Fatal, "FEngineLoop::Init requires no existing engine");
	}

	TUniquePtr<IModuleInterface> gameModule = CreateGameModule();
	if (!moduleManager.RegisterModule(gameModule))
	{
		SE_LOG(LogEngineLoop, Fatal, "Game module registration failed");
	}
	if (!moduleManager.LoadModule())
	{
		SE_LOG(LogEngineLoop, Fatal, "Game module loading failed");
	}

	engine = MakeUnique<UGameEngine>();
	engine->Init(this);

	engine->Start();
	previousTime = std::chrono::steady_clock::now();

	return 0;
}

void FEngineLoop::Tick()
{
	if (!engine) { return; }

	const auto currentTime = std::chrono::steady_clock::now();
	const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
	previousTime = currentTime;
	engine->Tick(deltaSeconds);
}

void FEngineLoop::Exit()
{
	if (engine)
	{
		engine->PreExit();
		engine.reset();
	}

	// 준비 전 / 반복 종료에서는 미등록 false도 허용한다.
	(void)moduleManager.UnloadModule();
}
