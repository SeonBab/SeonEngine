#include "ActorComponent.h"

FActorComponent::FActorComponent(AActor& owner)
	: owner(owner)
{
}

FActorComponent::~FActorComponent() = default;

bool FActorComponent::AllowsMultipleComponents() const
{
	return false;
}
