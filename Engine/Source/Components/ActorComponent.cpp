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
