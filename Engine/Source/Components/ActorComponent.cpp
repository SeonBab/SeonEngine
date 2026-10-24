#include "ActorComponent.h"

UActorComponent::UActorComponent(AActor& owner)
	: owner(owner)
{
}

UActorComponent::~UActorComponent() = default;

bool UActorComponent::AllowsMultipleComponents() const
{
	return false;
}

bool UActorComponent::HasBegunPlay() const
{
	return playState == EPlayState::Begun;
}

void UActorComponent::BeginPlay()
{
	playState = EPlayState::Begun;
}

void UActorComponent::EndPlay([[maybe_unused]] EEndPlayReason reason)
{
	playState = EPlayState::NotBegun;
}

void UActorComponent::TickComponent([[maybe_unused]] float deltaSeconds)
{
}
