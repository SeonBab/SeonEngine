#pragma once

#include "Core/Math/Matrix4.h"
#include "Core/Math/Quat.h"
#include "Core/Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 위치·회전·크기를 묶어 보관한다. 기본값은 위치 0·회전 없음·크기 1배다.
// 좌표 공간이나 부모 관계는 저장하지 않으며 크기·회전·이동 순서의 행렬을 만든다.
////////////////////////////////////////////////////////////////////////////////
struct FTransform
{
	FVector3 position{};
	FQuat rotation{};
	FVector3 scale{1.0f, 1.0f, 1.0f};

	/** 자신을 먼저, other를 나중에 적용한다. 유한한 값·정규화된 회전이 전제이며 음수 크기는 행렬 보조 경로를 사용한다. 원본은 유지한다. */
	[[nodiscard]] FTransform operator*(const FTransform& other) const;

	/** other 기준 상대값을 반환한다. 유한한 값·단위 회전이 전제이며 음수 크기는 행렬 보조 경로를 사용한다. 기준 크기의 절댓값<=1e-8은 역수 0, 기준 회전 검사 실패는 기본 Transform으로 대체한다. 원본은 유지한다. */
	[[nodiscard]] FTransform GetRelativeTransform(const FTransform& other) const;

	/** 저장된 회전의 길이 제곱과 1의 차이가 0.01 이하인지 반환한다. 원본은 유지한다. */
	[[nodiscard]] bool IsRotationNormalized() const;

	/** 유한한 위치·크기와 정규화된 회전으로 크기 → 회전 → 이동 행렬을 반환한다. 원본은 유지한다. */
	[[nodiscard]] FMatrix4 ToMatrixWithScale() const;

	/** 크기를 제외하고 유한한 위치와 정규화된 회전으로 회전 → 이동 행렬을 반환한다. 원본은 유지한다. */
	[[nodiscard]] FMatrix4 ToMatrixNoScale() const;
};
