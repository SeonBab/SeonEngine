#include "GameInstance.h"

UGameInstance::~UGameInstance() = default;

bool UGameInstance::Initialize(UWorld&, FGameClient&, IRenderer&)
{
	return true;
}
