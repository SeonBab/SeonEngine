#include "Object.h"

#include "Reflection/TypeInfo.h"

void UObject::PostLoad()
{
}

const FTypeInfo& UObject::StaticTypeInfo()
{
	static const FTypeInfo typeInfo("UObject", nullptr, {});

	return typeInfo;
}

const FTypeInfo& UObject::GetTypeInfo() const
{
	return StaticTypeInfo();
}
