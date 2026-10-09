#include "UObject/Object.h"

UObject::UObject(UObject* inOuter)
	: outer(inOuter)
{
}
