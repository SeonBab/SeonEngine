#include "SampleActor.h"

#include "Components/TransformComponent.h"
#include "Core/Math/Rotator.h"

#include <cmath>

ASampleActor::ASampleActor(UWorld& world)
	: AActor(world)
{
}

void ASampleActor::Tick(float deltaSeconds)
{
	elapsedSeconds += deltaSeconds;

	UTransformComponent* transform = GetComponent<UTransformComponent>();
	FRotator rotation = transform->GetRotation();
	rotation.yaw = 65.0f * std::sin(elapsedSeconds);
	transform->SetRotation(rotation);
}
