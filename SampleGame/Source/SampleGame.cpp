#include "Actor.h"
#include "ActorComponent.h"
#include "GameEntry.h"
#include "Renderer/StaticMeshData.h"
#include "SampleActor.h"
#include "StaticMeshComponent.h"
#include "Templates/UniquePtr.h"
#include "World.h"

#include <utility>

bool ConfigureGameWorld(UWorld& world)
{
	AActor& actor = world.SpawnActor<ASampleActor>();
	const FStaticMeshData meshData =
		{
			{{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
				{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
				{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}},
			{0, 1, 2, 0, 2, 3}};
	TUniquePtr<UStaticMeshComponent> mesh = MakeUnique<UStaticMeshComponent>(actor);
	mesh->SetMeshData(meshData);

	// 게임은 CPU 데이터와 소유 관계만 구성하고 렌더 상태 준비는 엔진에 맡긴다.
	TUniquePtr<UActorComponent> component = std::move(mesh);

	return actor.AddComponent(component);
}
