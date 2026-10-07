#include "ActorComponent.h"

FActorComponent::FActorComponent(FActor& owner)
	: owner(&owner)
{
}

FActorComponent::~FActorComponent() = default;

FActor& FActorComponent::GetOwner() const
{
	return *owner;
}
