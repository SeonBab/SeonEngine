#pragma once

#include "Core/Matrix4x4.h"
#include "Core/Quaternion.h"
#include "Core/Vector3.h"

/**
 * 한 카메라 시점의 원본 값 사본이다. 위치와 회전은 월드 기준이다.
 * 뷰 행렬과 투영 행렬은 여기서 계산하며 카메라 컴포넌트와 렌더러 어느 쪽에도 묶이지 않는다.
 */
struct CameraViewInfo
{
	Vector3 position;
	Quaternion rotation;
	float verticalFovDegrees = 0.0f;
	float nearClipDistance = 0.0f;
	float farClipDistance = 0.0f;

	/** 위치와 회전으로 월드 좌표를 카메라 기준 좌표로 옮기는 뷰 행렬을 계산한다. */
	Matrix4x4 CalculateViewMatrix() const;

	/**
	 * 화면 비율(가로를 세로로 나눈 값)로 원근 투영 행렬을 계산한다. 화면 비율이 0 이하이거나
	 * 시야각, near, far가 CameraComponent의 허용 범위를 벗어나면 출력값을 유지하고 false를 반환한다.
	 */
	bool CalculateProjectionMatrix(float aspectRatio, Matrix4x4& outProjectionMatrix) const;
};
