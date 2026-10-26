#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 파일에서 읽은 액터 하나의 초기 설정. 실행 중 액터를 소유하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FMapActorData
{
	FString type;
	FVector3 position{};
	FRotator rotation{}; // Pitch, Yaw, Roll 순서의 도 단위.
	FVector3 scale{1.0f, 1.0f, 1.0f};
};
