#include "SampleGameModule.h"

#include "Engine/GameEngine.h"
#include "GameModule.h"
#include "Logging/LogMacros.h"
#include "SampleGameInstance.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogSampleGameModule);
}

void FSampleGameModule::StartupModule()
{
	if (!UGameEngine::RegisterGameInstanceFactory(&CreateSampleGameInstance))
	{
		SE_LOG(LogSampleGameModule, Fatal, "Game instance factory registration failed");
	}
}

void FSampleGameModule::ShutdownModule()
{
	if (!UGameEngine::UnregisterGameInstanceFactory(&CreateSampleGameInstance))
	{
		SE_LOG(LogSampleGameModule, Error, "Game instance factory unregistration failed");
	}
}

TUniquePtr<IModuleInterface> CreateGameModule()
{
	return MakeUnique<FSampleGameModule>();
}
