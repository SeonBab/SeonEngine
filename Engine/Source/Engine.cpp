#include "Engine.h"

#include "World.h"

UEngine::UEngine() = default;
UEngine::~UEngine() = default;

bool UEngine::Initialize(FRenderer&)
{
	if (world) { return false; }

	world = MakeUnique<UWorld>();

	return true;
}

void UEngine::Start()
{
}
