#include "Actor.h"

#include "ActorComponent.h"
#include "TransformComponent.h"
#include "World.h"

#include <typeinfo>
#include <utility>

AActor::AActor(UWorld& inWorld)
	: world(&inWorld)
{
	components.push_back(MakeUnique<UTransformComponent>(*this));
}

AActor::~AActor() = default;

void AActor::Tick([[maybe_unused]] float deltaSeconds)
{
}

bool AActor::HasActorBegunPlay() const
{
	return actorHasBegunPlay == EActorBeginPlayState::HasBegunPlay;
}

bool AActor::IsActorBeginningPlay() const
{
	return actorHasBegunPlay == EActorBeginPlayState::BeginningPlay;
}

UWorld& AActor::GetWorld() const
{
	return *world;
}

AActor* AActor::GetParent() const
{
	return parent;
}

int32 AActor::GetChildCount() const
{
	return static_cast<int32>(children.size());
}

AActor* AActor::GetChild(int32 index) const
{
	if (index < 0 || static_cast<TArray<AActor*>::size_type>(index) >= children.size())
	{
		return nullptr;
	}

	return children[index];
}

bool AActor::SetParent(AActor* newParent, EActorTransformRule rule)
{
	return GetWorld().SetActorParent(this, newParent, rule);
}

bool AActor::Destroy()
{
	return GetWorld().DestroyActor(this);
}

bool AActor::IsActorBeingDestroyed() const
{
	return bActorIsBeingDestroyed;
}

bool AActor::AddComponent(TUniquePtr<UActorComponent>& component)
{
	if (!component || &component->GetOwner() != this)
	{
		return false;
	}

	if (!component->AllowsMultipleComponents())
	{
		const std::type_info& componentType = typeid(*component);

		for (const auto& existingComponent : components)
		{
			if (typeid(*existingComponent) == componentType)
			{
				return false;
			}
		}
	}

	components.push_back(std::move(component));

	return true;
}

void AActor::BeginPlay()
{
	actorHasBegunPlay = EActorBeginPlayState::HasBegunPlay;
}

void AActor::EndPlay([[maybe_unused]] EEndPlayReason reason)
{
	if (actorHasBegunPlay == EActorBeginPlayState::HasBegunPlay)
	{
		actorHasBegunPlay = EActorBeginPlayState::HasNotBegunPlay;
	}
}

void AActor::DispatchBeginPlay()
{
	if (actorHasBegunPlay != EActorBeginPlayState::HasNotBegunPlay) { return; }

	actorHasBegunPlay = EActorBeginPlayState::BeginningPlay;
	BeginPlay();
}

void AActor::RouteEndPlay(EEndPlayReason reason)
{
	if (HasActorBegunPlay())
	{
		EndPlay(reason);
	}
}
