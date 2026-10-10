#include "Launch/LaunchEngineLoop.h"

#include "Engine/GameEngine.h"
#include "GameProject.h"
#include "Logging/LogMacros.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogEngineLoop);
}

FEngineLoop::FEngineLoop() = default;
FEngineLoop::~FEngineLoop() = default;

int32 FEngineLoop::Init()
{
	if (engine || gameProject)
	{
		SE_LOG(LogEngineLoop, Fatal, "FEngineLoop::Init requires no existing engine or game project");
	}

	gameProject = CreateGameProject();
	if (!gameProject)
	{
		SE_LOG(LogEngineLoop, Fatal, "Creating a game project failed");
	}

	engine = MakeUnique<UGameEngine>();
	engine->Init(this);

	FWorld* world = engine->GetWorld();
	if (!world)
	{
		SE_LOG(LogEngineLoop, Fatal, "Initializing a game project requires a world");
	}
	if (!gameProject->InitializeWorld(*world))
	{
		SE_LOG(LogEngineLoop, Fatal, "Initializing the game world failed");
	}

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
	gameProject.reset();
}
