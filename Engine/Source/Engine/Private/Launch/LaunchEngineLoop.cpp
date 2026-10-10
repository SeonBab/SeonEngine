#include "Launch/LaunchEngineLoop.h"

#include "Engine/GameEngine.h"
#include "Logging/LogMacros.h"

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
	if (!engine) { return; }

	engine->PreExit();
	engine.reset();
}
