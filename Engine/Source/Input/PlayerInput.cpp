#include "PlayerInput.h"

#include "Components/InputComponent.h"
#include "Input/InputActionEvaluator.h"

#include <cstddef>
#include <utility>

#pragma region ActionState

bool UPlayerInput::RegisterAction(TSharedPtr<const UInputAction> action)
{
	if (bDispatchingActionEvents || !action) { return false; }

	const EInputActionValueType valueType = action->GetValueType();
	if (valueType != EInputActionValueType::Boolean && valueType != EInputActionValueType::Axis1D) { return false; }

	if (FindActionInstanceData(*action)) { return false; }

	actionInstances.emplace_back(std::move(action));

	return true;
}

bool UPlayerInput::EvaluateAction(const UInputAction& action, const UInputMappingContext& context, const FInputState& input)
{
	if (bDispatchingActionEvents) { return false; }

	const std::optional<FInputActionValue> evaluatedValue = EvaluateActionValue(action, context, input);
	if (!evaluatedValue) { return false; }

	return UpdateActionValue(action, *evaluatedValue);
}

bool UPlayerInput::EvaluateInputActions(const UInputMappingContext& context, const FInputState& input)
{
	if (bDispatchingActionEvents) { return false; }

	TArray<FInputActionInstance> evaluatedInstances = actionInstances;
	for (FInputActionInstance& instance : evaluatedInstances)
	{
		const std::optional<FInputActionValue> evaluatedValue = EvaluateActionValue(instance.GetSourceAction(), context, input);
		if (!evaluatedValue) { return false; }
		if (!instance.UpdateValue(*evaluatedValue)) { return false; }
	}

	for (std::size_t index = 0; index < actionInstances.size(); ++index)
	{
		actionInstances[index] = std::move(evaluatedInstances[index]);
	}

	return true;
}

bool UPlayerInput::UpdateActionValue(const UInputAction& action, const FInputActionValue& newValue)
{
	if (bDispatchingActionEvents) { return false; }

	for (FInputActionInstance& instance : actionInstances)
	{
		if (&instance.GetSourceAction() == &action) { return instance.UpdateValue(newValue); }
	}

	return false;
}

const FInputActionInstance* UPlayerInput::FindActionInstanceData(const UInputAction& action) const
{
	for (const FInputActionInstance& instance : actionInstances)
	{
		if (&instance.GetSourceAction() == &action) { return &instance; }
	}

	return nullptr;
}

#pragma endregion

#pragma region EventDispatch

void UPlayerInput::DispatchActionEvents(UInputComponent& inputComponent, float deltaSeconds)
{
	if (bDispatchingActionEvents) { return; }

	bDispatchingActionEvents = true;

	for (const FInputActionInstance& instance : actionInstances)
	{
		const UInputAction& action = instance.GetSourceAction();
		const FInputActionValue& value = instance.GetValue();
		switch (instance.GetTransitionResult())
		{
			case EInputActionTransitionResult::None:
				break;
			case EInputActionTransitionResult::StartedAndTriggered:
				inputComponent.DispatchActionEvent(action, EInputActionEvent::Started, value, deltaSeconds);
				inputComponent.DispatchActionEvent(action, EInputActionEvent::Triggered, value, deltaSeconds);
				break;
			case EInputActionTransitionResult::Triggered:
				inputComponent.DispatchActionEvent(action, EInputActionEvent::Triggered, value, deltaSeconds);
				break;
			case EInputActionTransitionResult::Completed:
				inputComponent.DispatchActionEvent(action, EInputActionEvent::Completed, value, deltaSeconds);
				break;
		}
	}

	bDispatchingActionEvents = false;
}

#pragma endregion
