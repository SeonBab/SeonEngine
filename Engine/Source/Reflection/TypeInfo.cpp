#include "TypeInfo.h"

#include "Reflection/PropertyInfo.h"

#pragma region Lifecycle

FTypeInfo::FTypeInfo(FStringView name, const FTypeInfo* baseType, std::span<const FPropertyInfo> properties)
	: name(name),
	  baseType(baseType),
	  properties(properties)
{
}

#pragma endregion

#pragma region Information

const FPropertyInfo* FTypeInfo::FindProperty(FStringView propertyName) const
{
	for (const FTypeInfo* type = this; type; type = type->baseType)
	{
		for (const FPropertyInfo& property : type->properties)
		{
			if (property.GetName() == propertyName) { return &property; }
		}
	}

	return nullptr;
}

#pragma endregion
