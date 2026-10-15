#pragma once

#include "Math/Matrix4.h"
#include "Math/Quat.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 위치·회전·크기를 묶어 보관한다. 기본값은 위치 0·회전 없음·크기 1배다.
// 좌표 공간이나 부모 관계는 저장하지 않으며 크기·회전·이동 순서의 행렬을 만든다.
////////////////////////////////////////////////////////////////////////////////
struct FTransform
{
	FVector3 position{};
	FQuat rotation{};
	FVector3 scale{1.0f, 1.0f, 1.0f};

	/** 유한한 위치·크기와 정규화된 회전으로 크기 → 회전 → 이동 행렬을 반환한다. 원본은 유지한다. */
	[[nodiscard]] FMatrix4 ToMatrixWithScale() const;
};
