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

	return 0;
}

void FEngineLoop::Exit()
{
	if (!engine) { return; }

	engine->PreExit();
	engine.reset();
}
