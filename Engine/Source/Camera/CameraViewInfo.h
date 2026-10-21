#pragma once

#include "Core/CoreTypes.h"
#include "Core/Math/Transform.h"

/** 카메라 시점의 투영 방식이다 */
enum class ECameraProjectionMode : uint8
{
	Perspective,
	Orthographic
};

/** 월드 공간의 위치, 회전과 투영 설정을 보관한다 */
struct FCameraViewInfo
{
	FTransform worldTransform{};
	ECameraProjectionMode projectionMode = ECameraProjectionMode::Perspective;
	float horizontalFovDegrees = 90.0f;
	float orthographicWidth = 4.0f;
	float nearClipDistance = 0.1f;
	float farClipDistance = 1000.0f;

	/** 월드 공간의 위치, 회전으로 뷰 행렬을 계산한다. 스케일은 사용하지 않는다 */
	[[nodiscard]] FMatrix4 GetViewMatrix() const;

	/** 유효한 투영 설정과 양수 화면 비율을 전제로 투영행렬을 반환한다 */
	[[nodiscard]] FMatrix4 GetProjectionMatrix(float aspectRatio) const;

	/** 뷰 행렬과 투영행렬을 순서대로 곱해 반환한다 */
	[[nodiscard]] FMatrix4 GetWorldToClipMatrix(float aspectRatio) const;
};
