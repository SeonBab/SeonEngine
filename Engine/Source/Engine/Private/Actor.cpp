#include "Actor.h"

#include "ActorComponent.h"
#include "TransformComponent.h"

FActor::FActor()
{
	components.push_back(MakeUnique<FTransformComponent>(*this));
}

FActor::~FActor() = default;
