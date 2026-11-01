#include "Input/InputState.h"

#include <algorithm>

bool FInputState::IsKeyDown(const FKey& key) const
{
	return key.IsValid() && std::find(pressedKeys.begin(), pressedKeys.end(), key) != pressedKeys.end();
}

void FInputState::SetKeyDown(const FKey& key, bool isDown)
{
	if (!key.IsValid()) { return; }

	const TArray<FKey>::iterator found = std::find(pressedKeys.begin(), pressedKeys.end(), key);
	if (isDown)
	{
		if (found == pressedKeys.end())
		{
			pressedKeys.push_back(key);
		}
	}
	else if (found != pressedKeys.end())
	{
		pressedKeys.erase(found);
	}
}

void FInputState::AddKeyEvent(FKeyEvent event)
{
	if (!event.key.IsValid()) { return; }

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
	mouseDelta = {};
	mouseWheelDelta = 0.0f;
}

void FInputState::AddMouseDelta(FVector2 delta)
{
	mouseDelta.x += delta.x;
	mouseDelta.y += delta.y;
}

FVector2 FInputState::GetMouseDelta() const
{
	return mouseDelta;
}

void FInputState::AddMouseWheelDelta(float delta)
{
	mouseWheelDelta += delta;
}

float FInputState::GetMouseWheelDelta() const
{
	return mouseWheelDelta;
}

void FInputState::Reset()
{
	pressedKeys.clear();

	ResetFrameState();
}
