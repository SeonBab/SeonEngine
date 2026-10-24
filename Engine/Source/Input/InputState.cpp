#include "Input/InputState.h"

#pragma region KeyState

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

#pragma endregion

#pragma region KeyEvents

void FInputState::AddKeyEvent(FKeyEvent event)
{
	const uint8 index = static_cast<uint8>(event.key);
	if (event.key == EKey::Invalid || index >= static_cast<uint8>(EKey::Count)) { return; }

	if (event.type != EKeyEventType::Pressed && event.type != EKeyEventType::Released && event.type != EKeyEventType::Repeat) { return; }

	keyEvents.push_back(event);
	SetKeyDown(event.key, event.type != EKeyEventType::Released);
}

const TArray<FKeyEvent>& FInputState::GetKeyEvents() const
{
	return keyEvents;
}

void FInputState::ResetFrameState()
{
	keyEvents.clear();
}

#pragma endregion

#pragma region Reset

void FInputState::Reset()
{
	for (bool& isDown : keyStates)
	{
		isDown = false;
	}

	ResetFrameState();
}

#pragma endregion
