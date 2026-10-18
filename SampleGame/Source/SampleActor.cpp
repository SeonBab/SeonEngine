#include "SampleActor.h"

#include "Components/TransformComponent.h"
#include "Core/Math/Rotator.h"

ASampleActor::ASampleActor(UWorld& world)
	: AActor(world)
{
}

void ASampleActor::Tick(float deltaSeconds)
{
	UTransformComponent* transform = GetComponent<UTransformComponent>();
	FRotator rotation = transform->GetRotation();
	rotation.roll += 45.0f * deltaSeconds;
	transform->SetRotation(rotation);
}
