#include "Input/Events.h"

FKeyEvent::FKeyEvent(const FKey& inKey, bool bInIsRepeat)
	: key(inKey),
	  bIsRepeat(bInIsRepeat)
{
}

FKey FKeyEvent::GetKey() const
{
	return key;
}

bool FKeyEvent::IsRepeat() const
{
	return bIsRepeat;
}
