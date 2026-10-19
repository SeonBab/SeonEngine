#include "InputComponent.h"

#include "Core/CoreTypes.h"
#include "Input/InputState.h"

#include <utility>

UInputComponent::UInputComponent(AActor& owner)
	: UActorComponent(owner)
{
}

bool UInputComponent::BindKey(EKey key, EInputTrigger trigger, FInputCallback callback)
{
	const uint8 index = static_cast<uint8>(key);
	if (bProcessingInput || key == EKey::Invalid || index >= static_cast<uint8>(EKey::Count) || !callback)
	{
		return false;
	}

	if (trigger != EInputTrigger::Held && trigger != EInputTrigger::Pressed) { return false; }

	keyBindings.push_back({key, trigger, std::move(callback)});

	return true;
}

void UInputComponent::ProcessInput(const FInputState& input, float deltaSeconds)
{
	if (bProcessingInput) { return; }

	bProcessingInput = true;

	for (const FKeyEvent& event : input.GetKeyEvents())
	{
		if (event.type != EKeyEventType::Pressed) { continue; }

		for (const FInputKeyBinding& binding : keyBindings)
		{
			if (binding.trigger == EInputTrigger::Pressed && binding.key == event.key)
			{
				binding.callback(deltaSeconds);
			}
		}
	}

	for (const FInputKeyBinding& binding : keyBindings)
	{
		if (binding.trigger == EInputTrigger::Held && input.IsKeyDown(binding.key))
		{
			binding.callback(deltaSeconds);
		}
	}

	bProcessingInput = false;
}
