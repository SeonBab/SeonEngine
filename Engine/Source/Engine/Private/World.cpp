#include "World.h"

#include "Actor.h"

#include <cassert>
#include <cstdlib>
#include <utility>

FWorld::FWorld() = default;
FWorld::~FWorld() = default;

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
