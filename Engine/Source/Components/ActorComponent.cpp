#include "ActorComponent.h"

UActorComponent::UActorComponent(AActor& owner)
	: owner(owner)
{
}

UActorComponent::~UActorComponent() = default;

FStringView UActorComponent::GetName() const
{
	return name;
}

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

void UActorComponent::SetComponentTickEnabled(bool enabled)
{
	bTickEnabled = enabled;
}

bool UActorComponent::IsComponentTickEnabled() const
{
	return bTickEnabled;
}

void UActorComponent::TickComponent([[maybe_unused]] float deltaSeconds)
{
}
