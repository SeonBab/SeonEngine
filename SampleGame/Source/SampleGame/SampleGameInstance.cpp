#include "SampleGameInstance.h"

#include "Actor.h"
#include "Logging/LogMacros.h"
#include "Renderer/PositionColorVertex.h"
#include "StaticMeshComponent.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogSampleGameInstance);
}

USampleGameInstance::USampleGameInstance(UEngine& inEngine)
	: UGameInstance(inEngine)
{
}

void USampleGameInstance::StartGameInstance()
{
	UGameInstance::StartGameInstance();

	FWorld* world = GetWorld();
	if (!world)
	{
		SE_LOG(LogSampleGameInstance, Fatal, "Starting SampleGame requires an initialized world");
	}

	FActor& actor = world->SpawnActor();
	TUniquePtr<FStaticMeshComponent> mesh = MakeUnique<FStaticMeshComponent>(actor);
	mesh->SetVertices({
		FPositionColorVertex{FVector3{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		FPositionColorVertex{FVector3{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		FPositionColorVertex{FVector3{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
	});

	if (!actor.AddComponent(mesh))
	{
		SE_LOG(LogSampleGameInstance, Fatal, "Adding the sample mesh component failed");
	}
}

TUniquePtr<UGameInstance> CreateSampleGameInstance(UEngine& inEngine)
{
	return MakeUnique<USampleGameInstance>(inEngine);
}
