#include "ComponentTypeRegistry.h"

#include "Components/ActorComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Logging/Log.h"
#include "Core/Math/Vector3.h"

#include <cmath>
#include <utility>
#include <variant>

namespace
{
	bool GetTransformPosition(const UActorComponent& component, FPropertyValue& outValue)
	{
		const UTransformComponent* transform = dynamic_cast<const UTransformComponent*>(&component);
		if (!transform) { return false; }

		outValue = transform->GetPosition();
		return true;
	}

	bool SetTransformPosition(UActorComponent& component, const FPropertyValue& value)
	{
		UTransformComponent* transform = dynamic_cast<UTransformComponent*>(&component);
		if (!transform) { return false; }

		const FVector3* position = std::get_if<FVector3>(&value);
		if (!position) { return false; }

		if (!std::isfinite(position->x) || !std::isfinite(position->y) || !std::isfinite(position->z)) { return false; }

		// 자손의 월드 변환도 갱신하도록 기존 setter를 사용한다.
		transform->SetPosition(*position);
		return true;
	}

	TUniquePtr<UActorComponent> CreateTransform(AActor& actor)
	{
		return MakeUnique<UTransformComponent>(actor);
	}
}

bool FComponentTypeRegistry::RegisterBuiltInTypes()
{
	if (!Register("Transform", &CreateTransform, typeid(UTransformComponent))) { return false; }

	return RegisterProperty("Transform", {"position", &GetTransformPosition, &SetTransformPosition});
}

bool FComponentTypeRegistry::Register(FStringView typeName, FComponentFactory factory, const std::type_info& componentType)
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
	entries.push_back({FString(typeName), factory, &componentType, {}});
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
		if (entry.typeName == typeName) { return *entry.componentType == typeid(component); }
	}

	return false;
}

bool FComponentTypeRegistry::RegisterProperty(FStringView typeName, FComponentPropertyInfo property)
{
	if (property.name.empty())
	{
		WriteLog("Map", ELogLevel::Error, "Property name for component type '{}' must not be empty.", typeName);
		return false;
	}

	if (!property.getValue && !property.setValue)
	{
		WriteLog("Map", ELogLevel::Error, "Property '{}' for component type '{}' must provide an accessor.", property.name, typeName);
		return false;
	}

	for (FEntry& entry : entries)
	{
		if (entry.typeName != typeName) { continue; }

		for (const FComponentPropertyInfo& existing : entry.properties)
		{
			if (existing.name == property.name)
			{
				WriteLog("Map", ELogLevel::Error, "Property '{}' for component type '{}' is already registered.", property.name, typeName);
				return false;
			}
		}

		entry.properties.push_back(std::move(property));
		return true;
	}

	WriteLog("Map", ELogLevel::Error, "Cannot register property '{}' for unknown component type '{}'.", property.name, typeName);
	return false;
}

std::span<const FComponentPropertyInfo> FComponentTypeRegistry::GetProperties(FStringView typeName) const
{
	for (const FEntry& entry : entries)
	{
		if (entry.typeName == typeName) { return entry.properties; }
	}

	return {};
}
