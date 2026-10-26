#include "Actor.h"

#include "Components/ActorComponent.h"
#include "Components/TransformComponent.h"
#include "World.h"

#include <typeinfo>
#include <utility>

#pragma region Lifecycle

AActor::AActor(UWorld& inWorld)
	: world(&inWorld)
{
	TUniquePtr<UActorComponent> transform = MakeUnique<UTransformComponent>(*this);
	(void)AddComponent("RootTransform", transform);
}

AActor::~AActor() = default;

#pragma endregion

#pragma region WorldAndHierarchy

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

#pragma endregion

#pragma region Destruction

bool AActor::Destroy()
{
	return GetWorld().DestroyActor(this);
}

bool AActor::IsActorBeingDestroyed() const
{
	return bActorIsBeingDestroyed;
}

#pragma endregion

#pragma region PlayLifecycle

void AActor::SetActorTickEnabled(bool enabled)
{
	bTickEnabled = enabled;
}

bool AActor::IsActorTickEnabled() const
{
	return bTickEnabled;
}

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

#pragma endregion

#pragma region Components

bool AActor::AddComponent(FStringView name, TUniquePtr<UActorComponent>& component)
{
	if (name.empty() || FindComponent(name) || !component || &component->GetOwner() != this)
	{
		return false;
	}

	if (!component->AllowsMultipleComponents())
	{
		const std::type_info& componentType = typeid(*component);

		for (const TUniquePtr<UActorComponent>& existingComponent : components)
		{
			if (typeid(*existingComponent) == componentType)
			{
				return false;
			}
		}
	}

	// 모든 거부 조건을 통과한 뒤 이름과 소유권을 확정한다.
	component->name = FString(name);
	UActorComponent* addedComponent = component.get();
	components.push_back(std::move(component));

	if ((IsActorBeginningPlay() || HasActorBegunPlay()) && !addedComponent->HasBegunPlay())
	{
		addedComponent->BeginPlay();
	}

	return true;
}

UActorComponent* AActor::FindComponent(FStringView name) const
{
	for (const TUniquePtr<UActorComponent>& component : components)
	{
		if (component->GetName() == name) { return component.get(); }
	}

	return nullptr;
}

#pragma endregion
