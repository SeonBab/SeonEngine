#include "InputComponent.h"

#include "Core/CoreTypes.h"
#include "Input/InputState.h"

#include <utility>

UInputComponent::UInputComponent(AActor& owner)
	: UActorComponent(owner)
{
}

bool UInputComponent::BindKey(EKey key, FInputCallback callback)
{
	const uint8 index = static_cast<uint8>(key);
	if (bProcessingInput || key == EKey::Invalid || index >= static_cast<uint8>(EKey::Count) || !callback)
	{
		return false;
	}

	keyBindings.push_back({key, std::move(callback)});

	return true;
}

void UInputComponent::ProcessInput(const FInputState& input, float deltaSeconds)
{
	if (bProcessingInput) { return; }

	bProcessingInput = true;

	for (const FInputKeyBinding& binding : keyBindings)
	{
		if (input.IsKeyDown(binding.key))
		{
			binding.callback(deltaSeconds);
		}
	}

	bProcessingInput = false;
}
