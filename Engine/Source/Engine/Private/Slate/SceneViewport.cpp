#include "Slate/SceneViewport.h"

#include "Input/Events.h"
#include "InputCoreTypes.h"

bool FSceneViewport::KeyState(FKey key) const
{
	const bool* state = keyStateMap.Find(key);
	if (state == nullptr)
	{
		return false;
	}

	return *state;
}

FReply FSceneViewport::OnKeyDown(const FKeyEvent& inKeyEvent)
{
	const FKey key = inKeyEvent.GetKey();
	if (key == EKeys::Invalid)
	{
		return FReply::Unhandled();
	}

	keyStateMap.Add(key, true);

	return FReply::Handled();
}

FReply FSceneViewport::OnKeyUp(const FKeyEvent& inKeyEvent)
{
	const FKey key = inKeyEvent.GetKey();
	if (key == EKeys::Invalid)
	{
		return FReply::Unhandled();
	}

	keyStateMap.Add(key, false);

	return FReply::Handled();
}
