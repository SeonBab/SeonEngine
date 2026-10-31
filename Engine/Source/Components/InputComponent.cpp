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

bool UInputComponent::BindAction(TSharedPtr<const UInputAction> action, EInputActionEvent event, FInputActionCallback callback)
{
	if (bProcessingInput || !action || !callback) { return false; }
	if (event != EInputActionEvent::Started && event != EInputActionEvent::Triggered && event != EInputActionEvent::Completed) { return false; }

	actionBindings.push_back({std::move(action), event, std::move(callback)});

	return true;
}

void UInputComponent::DispatchActionEvent(const UInputAction& action, EInputActionEvent event, const FInputActionValue& value, float deltaSeconds)
{
	if (bProcessingInput) { return; }
	if (event != EInputActionEvent::Started && event != EInputActionEvent::Triggered && event != EInputActionEvent::Completed) { return; }

	bProcessingInput = true;

	for (const FInputActionBinding& binding : actionBindings)
	{
		if (binding.action.get() == &action && binding.event == event)
		{
			binding.callback(value, deltaSeconds);
		}
	}

	bProcessingInput = false;
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
