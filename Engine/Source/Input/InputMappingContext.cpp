#include "InputMappingContext.h"

#include <cmath>
#include <utility>

bool UInputMappingContext::MapKey(TSharedPtr<const UInputAction> action, EKey key, float scale)
{
	const uint8 keyIndex = static_cast<uint8>(key);
	if (!action || key == EKey::Invalid || keyIndex >= static_cast<uint8>(EKey::Count) || !std::isfinite(scale))
	{
		return false;
	}

	mappings.push_back({std::move(action), key, scale});

	return true;
}
