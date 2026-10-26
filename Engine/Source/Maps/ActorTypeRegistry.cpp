#include "ActorTypeRegistry.h"

#include "Actor.h"
#include "Core/Logging/Log.h"
#include "World.h"

namespace
{
	AActor& CreateActor(UWorld& world)
	{
		// 직접 할당하지 않고 World의 소유 목록에 등록하는 경로를 사용한다.
		return world.SpawnActor<AActor>();
	}
}

bool FActorTypeRegistry::RegisterBuiltInTypes()
{
	return Register("Actor", &CreateActor);
}

bool FActorTypeRegistry::Register(FStringView typeName, FActorFactory factory)
{
	if (typeName.empty())
	{
		WriteLog("Map", ELogLevel::Error, "Actor type name must not be empty.");
		return false;
	}

	if (!factory)
	{
		WriteLog("Map", ELogLevel::Error, "Factory for actor type '{}' must not be null.", typeName);
		return false;
	}

	if (Find(typeName))
	{
		WriteLog("Map", ELogLevel::Error, "Actor type '{}' is already registered.", typeName);
		return false;
	}

	// 조회 시 원본 문자열의 수명에 의존하지 않도록 이름을 소유한다.
	entries.push_back({FString(typeName), factory});

	return true;
}

FActorFactory FActorTypeRegistry::Find(FStringView typeName) const
{
	for (const FEntry& entry : entries)
	{
		if (entry.typeName == typeName)
		{
			return entry.factory;
		}
	}

	return nullptr;
}
