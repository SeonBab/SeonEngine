#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

/** 카메라 공간 좌표를 화면으로 옮기는 방식이다. */
enum class CameraProjectionMode : unsigned char
{
	// 멀수록 작게 보인다. verticalFovDegrees를 쓴다.
	Perspective,
	// 깊이와 무관하게 같은 크기로 보인다. orthographicHeight를 쓴다.
	Orthographic
};

/**
 * 한 카메라 시점의 원본 값 사본이다. 위치와 회전은 월드 기준이다.
 * 뷰 행렬과 투영 행렬은 여기서 계산하며 카메라 컴포넌트와 렌더러 어느 쪽에도 묶이지 않는다.
 */
struct CameraViewInfo
{
	Vector3 position;
	Quaternion rotation;
	CameraProjectionMode projectionMode = CameraProjectionMode::Perspective;
	/** 원근 모드에서 쓰는 세로 시야각이며 단위는 도다. */
	float verticalFovDegrees = 0.0f;
	/** 직교 모드에서 보이는 영역의 전체 세로 길이이며 단위는 cm다. 가로는 이 값 * 화면 비율이다. */
	float orthographicHeight = 0.0f;
	/** 두 모드가 함께 쓰는 그리기 거리이며 단위는 cm다. */
	float nearClipDistance = 0.0f;
	float farClipDistance = 0.0f;

	/** 위치와 회전으로 월드 좌표를 카메라 기준 좌표로 옮기는 뷰 행렬을 계산한다. */
	Matrix4x4 CalculateViewMatrix() const;

	/**
	 * 화면 비율(가로를 세로로 나눈 값)과 투영 모드에 맞는 투영 행렬을 계산한다.
	 * 화면 비율이 0 이하이거나, near와 far 또는 현재 모드의 시야각이나 직교 높이가
	 * 허용 범위를 벗어나면 출력값을 유지하고 false를 반환한다.
	 */
	bool CalculateProjectionMatrix(float aspectRatio, Matrix4x4& outProjectionMatrix) const;
};
