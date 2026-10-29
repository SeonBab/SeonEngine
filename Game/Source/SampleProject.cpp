#include "Actor.h"
#include "Maps/ActorTypeRegistry.h"
#include "Maps/GameMapsSettings.h"
#include "Maps/GameTypeRegistration.h"
#include "SampleActor.h"
#include "World.h"

namespace
{
	AActor& CreateSampleActor(UWorld& world)
	{
		return world.SpawnActor<ASampleActor>();
	}
}

const FGameMapsSettings& GetGameMapsSettings()
{
	static const FGameMapsSettings settings{
		"/Game/Maps/SampleDev.json",
		"/Game/Maps/SampleDev.json"};

	return settings;
}

bool RegisterGameTypes(FActorTypeRegistry& actorTypes, FComponentTypeRegistry&)
{
	return actorTypes.Register("SampleActor", &CreateSampleActor);
}
