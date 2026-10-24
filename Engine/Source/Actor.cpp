#include "Actor.h"

#include "Components/ActorComponent.h"
#include "Components/TransformComponent.h"
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
	return actorHasBegunPlay == EPlayState::Begun;
}

bool AActor::IsActorBeginningPlay() const
{
	return actorHasBegunPlay == EPlayState::Beginning;
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

	UActorComponent* addedComponent = component.get();
	components.push_back(std::move(component));

	if ((IsActorBeginningPlay() || HasActorBegunPlay()) && !addedComponent->HasBegunPlay())
	{
		addedComponent->BeginPlay();
	}

	return true;
}

void AActor::BeginPlay()
{
	TArray<UActorComponent*> actorComponents;
	GetComponents(actorComponents);

	for (UActorComponent* component : actorComponents)
	{
		if (component->HasBegunPlay())
		{
			continue;
		}

		component->BeginPlay();
	}

	actorHasBegunPlay = EPlayState::Begun;
}

void AActor::EndPlay(EEndPlayReason reason)
{
	if (actorHasBegunPlay != EPlayState::Begun)
	{
		return;
	}

	actorHasBegunPlay = EPlayState::NotBegun;

	TArray<UActorComponent*> actorComponents;
	GetComponents(actorComponents);

	for (UActorComponent* component : actorComponents)
	{
		if (!component->HasBegunPlay())
		{
			continue;
		}

		component->EndPlay(reason);
	}
}

void AActor::DispatchBeginPlay()
{
	if (actorHasBegunPlay != EPlayState::NotBegun) { return; }

	actorHasBegunPlay = EPlayState::Beginning;
	BeginPlay();
}

void AActor::RouteEndPlay(EEndPlayReason reason)
{
	if (HasActorBegunPlay())
	{
		EndPlay(reason);
	}
}
