#include "ActorComponent.h"

#pragma region Lifecycle

UActorComponent::UActorComponent(AActor& owner)
	: owner(owner)
{
}

UActorComponent::~UActorComponent() = default;

#pragma endregion

#pragma region ActorIntegration

bool UActorComponent::AllowsMultipleComponents() const
{
	return false;
}

#pragma endregion

#pragma region PlayLifecycle

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

#pragma endregion
