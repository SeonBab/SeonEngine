#include "InputActionEvaluator.h"

#include "Input/InputAction.h"
#include "Input/InputMappingContext.h"
#include "Input/InputState.h"

#include <cmath>
#include <limits>

std::optional<FInputActionValue> EvaluateActionValue(const UInputAction& action, const UInputMappingContext& context, const FInputState& input)
{
	const EInputActionValueType valueType = action.GetValueType();
	if (valueType != EInputActionValueType::Boolean && valueType != EInputActionValueType::Axis1D) { return std::nullopt; }

	const EInputActionAccumulationBehavior accumulationBehavior = action.GetAccumulationBehavior();
	if (accumulationBehavior != EInputActionAccumulationBehavior::Cumulative && accumulationBehavior != EInputActionAccumulationBehavior::TakeHighestAbsoluteValue)
	{
		return std::nullopt;
	}

	double combinedValue = 0.0;
	for (const FInputActionKeyMapping& mapping : context.GetMappings())
	{
		if (mapping.action.get() != &action || !input.IsKeyDown(mapping.key)) { continue; }

		switch (accumulationBehavior)
		{
			case EInputActionAccumulationBehavior::Cumulative:
				combinedValue += mapping.scale;
				break;

			case EInputActionAccumulationBehavior::TakeHighestAbsoluteValue:
				if (std::abs(mapping.scale) >= std::abs(combinedValue))
				{
					combinedValue = mapping.scale;
				}
				break;
		}
	}

	const double maxValue = std::numeric_limits<float>::max();
	if (!std::isfinite(combinedValue) || combinedValue < -maxValue || combinedValue > maxValue) { return std::nullopt; }

	if (valueType == EInputActionValueType::Boolean)
	{
		return FInputActionValue(combinedValue != 0.0);
	}

	return FInputActionValue(static_cast<float>(combinedValue));
}

EInputActionTransitionResult EvaluateActionTransition(EInputActionState previousState, EInputActionState currentState)
{
	if (currentState == EInputActionState::Active)
	{
		if (previousState == EInputActionState::Inactive) { return EInputActionTransitionResult::StartedAndTriggered; }

		return EInputActionTransitionResult::Triggered;
	}

	if (previousState == EInputActionState::Active) { return EInputActionTransitionResult::Completed; }

	return EInputActionTransitionResult::None;
}
