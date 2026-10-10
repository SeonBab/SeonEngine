#include "Modules/ModuleManager.h"

#include "Modules/ModuleInterface.h"

FModuleManager::FModuleManager() = default;
FModuleManager::~FModuleManager() = default;

bool FModuleManager::RegisterModule(TUniquePtr<IModuleInterface>& inModule)
{
	if (!inModule || module)
	{
		return false;
	}

	module = std::move(inModule);

	return true;
}

IModuleInterface* FModuleManager::LoadModule()
{
	if (!module)
	{
		return nullptr;
	}

	module->StartupModule();

	return module.get();
}

bool FModuleManager::UnloadModule()
{
	if (!module)
	{
		return false;
	}

	module->ShutdownModule();
	module.reset();

	return true;
}
