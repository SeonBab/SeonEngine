#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 로컬 위치·선형 RGBA 색상·법선·UV 좌표를 보관한다. GPU 자원은 소유하지 않는다.
// 기본값은 원점·불투명한 흰색·0 법선·UV (0, 0)이며, 0 법선은 유효한 표면 방향이 아니다.
////////////////////////////////////////////////////////////////////////////////
struct FVertex
{
	FVector3 position{};
	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	FVector3 normal{};
	FVector2 uv{};
};
