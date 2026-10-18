#include "Input/InputState.h"

bool FInputState::IsKeyDown(EKey key) const
{
	const uint8 index = static_cast<uint8>(key);
	if (key == EKey::Invalid || index >= static_cast<uint8>(EKey::Count))
	{
		return false;
	}

	return keyStates[index];
}

void FInputState::SetKeyDown(EKey key, bool isDown)
{
	const uint8 index = static_cast<uint8>(key);
	if (key == EKey::Invalid || index >= static_cast<uint8>(EKey::Count))
	{
		return;
	}

	keyStates[index] = isDown;
}

void FInputState::Reset()
{
	for (bool& isDown : keyStates)
	{
		isDown = false;
	}
}
