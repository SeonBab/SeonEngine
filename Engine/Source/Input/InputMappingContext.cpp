#include "InputMappingContext.h"

#include <cmath>
#include <utility>

bool UInputMappingContext::MapKey(TSharedPtr<const UInputAction> action, const FKey& key, float scale)
{
	if (!action || !key.IsValid() || !std::isfinite(scale))
	{
		return false;
	}

	mappings.push_back({std::move(action), key, scale});

	return true;
}
