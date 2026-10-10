#include "World.h"

#include "Actor.h"
#include "Renderer/MeshRenderData.h"
#include "StaticMeshComponent.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <utility>

FWorld::FWorld() = default;
FWorld::~FWorld() = default;

void FWorld::Tick(float deltaSeconds)
{
	if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.0f)
	{
		assert(false && "World Tick requires a finite non-negative deltaSeconds");
		std::abort();
	}

	DeltaTimeSeconds = deltaSeconds;
	TimeSeconds += static_cast<double>(deltaSeconds);

	TArray<FActor*> actorsToTick;
	CollectActors(actorsToTick);
	for (FActor* actor : actorsToTick)
	{
		if (actor->IsActorBeingDestroyed()) { continue; }

		actor->Tick(deltaSeconds);
	}
}

void FWorld::CollectActors(TArray<FActor*>& outActors) const
{
	outActors.clear();
	for (const TUniquePtr<FActor>& actor : actors)
	{
		outActors.push_back(actor.get());
	}

	for (TArray<FActor*>::size_type index = 0; index < outActors.size(); ++index)
	{
		FActor* actor = outActors[index];
		for (const TUniquePtr<FActor>& child : actor->children)
		{
			outActors.push_back(child.get());
		}
	}
}

void FWorld::CollectStaticMeshComponents(TArray<FStaticMeshComponent*>& outComponents) const
{
	outComponents.clear();
	TArray<FActor*> worldActors;
	CollectActors(worldActors);
	TArray<FStaticMeshComponent*> actorComponents;
	for (FActor* actor : worldActors)
	{
		if (actor->IsActorBeingDestroyed()) { continue; }

		actor->GetComponents(actorComponents);
		outComponents.insert(outComponents.end(), actorComponents.begin(), actorComponents.end());
	}
}

void FWorld::CollectMeshRenderData(TArray<FMeshRenderData>& outRenderData) const
{
	outRenderData.clear();
	TArray<FStaticMeshComponent*> meshComponents;
	CollectStaticMeshComponents(meshComponents);
	outRenderData.reserve(meshComponents.size());
	for (const FStaticMeshComponent* meshComponent : meshComponents)
	{
		outRenderData.push_back(meshComponent->GetRenderData());
	}
}

float FWorld::GetDeltaSeconds() const
{
	return DeltaTimeSeconds;
}

double FWorld::GetTimeSeconds() const
{
	return TimeSeconds;
}

FActor& FWorld::SpawnActor()
{
	actors.push_back(MakeUnique<FActor>(*this));
	return *actors.back();
}

void FWorld::SetGameInstance(UGameInstance* inGameInstance)
{
	gameInstance = inGameInstance;
}

bool FWorld::DestroyActor(FActor* actor)
{
	// 호출 전제 위반은 일반적인 제거 거부가 아니다. Release에서도 중단하여 소유 목록을 보호한다.
	if (!actor || actor->world != this)
	{
		assert(false && "DestroyActor requires a live actor from this world");
		std::abort();
	}
	if (actor->IsActorBeingDestroyed()) { return true; }

	TArray<TUniquePtr<FActor>>& currentOwners = actor->parent ? actor->parent->children : actors;
	TArray<TUniquePtr<FActor>>::iterator ownedActor = currentOwners.begin();
	while (ownedActor != currentOwners.end() && ownedActor->get() != actor)
	{
		++ownedActor;
	}
	if (ownedActor == currentOwners.end())
	{
		assert(false && "DestroyActor requires a world-owned actor");
		std::abort();
	}

	// 상태 변경 후 할당을 피하도록 보관 목록의 저장 공간을 먼저 준비한다.
	if (destroyedActors.size() == destroyedActors.capacity())
	{
		constexpr TArray<TUniquePtr<FActor>>::size_type growthFactor = 2;
		destroyedActors.reserve(destroyedActors.empty() ? 1 : destroyedActors.size() * growthFactor);
	}
	MarkActorIsBeingDestroyed(*actor);
	TUniquePtr<FActor> owned = std::move(*ownedActor);
	currentOwners.erase(ownedActor);
	destroyedActors.push_back(std::move(owned));
	actor->parent = nullptr;

	return true;
}

void FWorld::MarkActorIsBeingDestroyed(FActor& actor)
{
	actor.bActorIsBeingDestroyed = true;
	for (const TUniquePtr<FActor>& child : actor.children)
	{
		MarkActorIsBeingDestroyed(*child);
	}
}

void FWorld::PurgeDestroyedActors()
{
	destroyedActors.clear();
}
