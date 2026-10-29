#include "ComponentTypeRegistry.h"

#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Logging/Log.h"

namespace
{
	TUniquePtr<UActorComponent> CreateTransform(AActor& actor)
	{
		return MakeUnique<UTransformComponent>(actor);
	}

	TUniquePtr<UActorComponent> CreateDirectionalLight(AActor& actor)
	{
		return MakeUnique<UDirectionalLightComponent>(actor);
	}

	TUniquePtr<UActorComponent> CreateSkyLight(AActor& actor)
	{
		return MakeUnique<USkyLightComponent>(actor);
	}

	TUniquePtr<UActorComponent> CreateStaticMesh(AActor& actor)
	{
		return MakeUnique<UStaticMeshComponent>(actor);
	}
}

bool FComponentTypeRegistry::RegisterBuiltInTypes()
{
	if (!Register("Transform", &CreateTransform, typeid(UTransformComponent), UTransformComponent::StaticTypeInfo())) { return false; }
	if (!Register("DirectionalLight", &CreateDirectionalLight, typeid(UDirectionalLightComponent), UDirectionalLightComponent::StaticTypeInfo())) { return false; }

	if (!Register("SkyLight", &CreateSkyLight, typeid(USkyLightComponent), USkyLightComponent::StaticTypeInfo())) { return false; }

	return Register("StaticMesh", &CreateStaticMesh, typeid(UStaticMeshComponent), UStaticMeshComponent::StaticTypeInfo());
}

bool FComponentTypeRegistry::Register(FStringView typeName, FComponentFactory factory, const std::type_info& componentType, const FTypeInfo& typeInfo)
{
	if (typeName.empty())
	{
		WriteLog("Map", ELogLevel::Error, "Component type name must not be empty.");
		return false;
	}

	if (!factory)
	{
		WriteLog("Map", ELogLevel::Error, "Factory for component type '{}' must not be null.", typeName);
		return false;
	}

	if (Find(typeName))
	{
		WriteLog("Map", ELogLevel::Error, "Component type '{}' is already registered.", typeName);
		return false;
	}

	// 조회 시 원본 문자열의 수명에 의존하지 않도록 이름을 소유한다.
	entries.push_back({FString(typeName), factory, &componentType, &typeInfo});
	return true;
}

FComponentFactory FComponentTypeRegistry::Find(FStringView typeName) const
{
	for (const FEntry& entry : entries)
	{
		if (entry.typeName == typeName) { return entry.factory; }
	}

	return nullptr;
}

bool FComponentTypeRegistry::IsType(FStringView typeName, const UActorComponent& component) const
{
	for (const FEntry& entry : entries)
	{
		if (entry.typeName == typeName) { return *entry.componentType == typeid(component) && entry.typeInfo == &component.GetTypeInfo(); }
	}

	return false;
}

const FTypeInfo* FComponentTypeRegistry::FindTypeInfo(FStringView typeName) const
{
	for (const FEntry& entry : entries)
	{
		if (entry.typeName == typeName) { return entry.typeInfo; }
	}

	return nullptr;
}
