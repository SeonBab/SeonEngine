#include "InputActionInstance.h"

#include <cmath>

bool FInputActionInstance::UpdateValue(const FInputActionValue& newValue)
{
	if (newValue.GetValueType() != action->GetValueType()) { return false; }
	if (!std::isfinite(newValue.GetAxis1D())) { return false; }

	EInputActionState currentState = EInputActionState::Inactive;
	if (newValue.GetBoolean())
	{
		currentState = EInputActionState::Active;
	}

	const EInputActionTransitionResult result = EvaluateActionTransition(state, currentState);
	value = newValue;
	state = currentState;
	transitionResult = result;

	return true;
}
