#include "Engine.h"

#include "World.h"

#pragma region Lifecycle

UEngine::UEngine() = default;

UEngine::~UEngine() = default;

#pragma endregion

#pragma region Execution

bool UEngine::Initialize(FRenderer&)
{
	if (world) { return false; }

	world = MakeUnique<UWorld>();

	return true;
}

void UEngine::Start()
{
}

#pragma endregion
