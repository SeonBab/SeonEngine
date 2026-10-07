#pragma once

#include "Math/Matrix4.h"
#include "Math/Quat.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 위치, 회전, 크기를 묶어 저장하는 트랜스폼이다.
// 기본값은 위치 0, 회전 없음, 크기 1배이며 좌표 공간이나 부모 관계는 저장하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FTransform
{
	/** 위치. X 왼쪽 / Y 위 / Z 앞이며 단위는 cm다. */
	FVector3 position{};
	/** 회전. 기본값은 회전하지 않은 상태다. */
	FQuat rotation{};
	/** 축별 크기 배율. 기본값은 각 축 1배다. */
	FVector3 scale{1.0f, 1.0f, 1.0f};

	/** 유한한 위치 / 크기와 단위 회전으로 크기 -> 회전 -> 이동 순서의 행렬을 만든다. */
	[[nodiscard]] FMatrix4 ToMatrixWithScale() const;

	/** 크기를 제외하고 유한한 위치와 단위 회전으로 회전 -> 이동 순서의 행렬을 만든다. */
	[[nodiscard]] FMatrix4 ToMatrixNoScale() const;
};
