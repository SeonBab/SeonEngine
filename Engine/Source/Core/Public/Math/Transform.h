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

	/** 자신을 먼저, other를 나중에 적용하는 합성이다. 유한한 값 / 단위 회전을 전제로 하며 음수 크기는 행렬 경로를 사용한다. */
	[[nodiscard]] FTransform operator*(const FTransform& other) const;

	/** 저장된 회전의 길이 제곱과 1의 차이가 0.01 이하인지 반환한다. 유한한 성분이 전제다. */
	[[nodiscard]] bool IsRotationNormalized() const;

	/** other 기준의 상대 값을 반환한다. 유한한 값 / 단위 회전이 전제이며 기준 회전이 정규화 검사를 통과하지 못하면 전체 항등값을 반환한다. 음수 크기는 행렬 경로, 기준 축 크기 절댓값 1e-8 이하는 안전 역수 0, 역행렬 불가는 항등 대체를 사용한다. */
	[[nodiscard]] FTransform GetRelativeTransform(const FTransform& other) const;

	/** 유한한 위치 / 크기와 단위 회전으로 크기 -> 회전 -> 이동 순서의 행렬을 만든다. */
	[[nodiscard]] FMatrix4 ToMatrixWithScale() const;

	/** 크기를 제외하고 유한한 위치와 단위 회전으로 회전 -> 이동 순서의 행렬을 만든다. */
	[[nodiscard]] FMatrix4 ToMatrixNoScale() const;
};
