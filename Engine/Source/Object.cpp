#include "Object.h"

#include "Reflection/TypeInfo.h"

#pragma region Lifecycle

void UObject::PostLoad()
{
}

#pragma endregion

#pragma region Reflection

const FTypeInfo& UObject::StaticTypeInfo()
{
	static const FTypeInfo typeInfo("UObject", nullptr, {});

	return typeInfo;
}

const FTypeInfo& UObject::GetTypeInfo() const
{
	return StaticTypeInfo();
}

#pragma endregion
