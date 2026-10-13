#include "InputComponent.h"

FInputComponent::FInputComponent(FActor& ownerActor)
	: FActorComponent(ownerActor)
{
}

bool FInputComponent::IsPressed(FKey inKey) const
{
	const bool* state = keyStateMap.Find(inKey);
	return state ? *state : false;
}
