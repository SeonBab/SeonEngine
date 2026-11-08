#pragma once

#include "CameraViewInfo.h"
#include "Component.h"

/** 원근 또는 직교 투영 카메라의 설정값을 보관한다. 위치와 회전은 같은 객체의 TransformComponent를 따른다. */
class CameraComponent : public Component
{
	// 투영 모드
public:
	/** 기본값은 원근이다. 모드를 바꿔도 각 모드의 설정값은 유지된다. */
	CameraProjectionMode GetProjectionMode() const { return projectionMode; }
	void SetProjectionMode(CameraProjectionMode newProjectionMode) { projectionMode = newProjectionMode; }

private:
	CameraProjectionMode projectionMode = CameraProjectionMode::Perspective;

	// 원근 투영 설정
public:
	/** 세로 시야각이며 단위는 도다. */
	float GetVerticalFovDegrees() const { return verticalFovDegrees; }

	/** 0보다 크고 180보다 작은 값만 받는다. 범위를 벗어나면 기존 값을 유지하고 false를 반환한다. */
	bool SetVerticalFovDegrees(float newVerticalFovDegrees);

private:
	float verticalFovDegrees = 60.0f;

	// 직교 투영 설정
public:
	/** 보이는 영역의 전체 세로 길이이며 단위는 cm다. 가로는 이 값 * 화면 비율이다. */
	float GetOrthographicHeight() const { return orthographicHeight; }

	/** 0보다 큰 값만 받는다. 아니면 기존 값을 유지하고 false를 반환한다. */
	bool SetOrthographicHeight(float newOrthographicHeight);

private:
	float orthographicHeight = 1000.0f;

	// 그리기 거리 (두 모드 공통)
public:
	/** 가까운 쪽 그리기 경계까지의 거리이며 단위는 cm다. */
	float GetNearClipDistance() const { return nearClipDistance; }

	/** 먼 쪽 그리기 경계까지의 거리이며 단위는 cm다. */
	float GetFarClipDistance() const { return farClipDistance; }

	/**
	 * near와 far를 함께 설정한다. 0 < near < far일 때만 받고,
	 * 아니면 기존 값을 유지하고 false를 반환한다.
	 */
	bool SetClipDistances(float newNearClipDistance, float newFarClipDistance);

private:
	float nearClipDistance = 10.0f;
	float farClipDistance = 100000.0f;

	// 시점 값
public:
	/**
	 * 같은 객체의 TransformComponent 위치와 회전, 이 카메라의 투영 설정으로 시점 값을 채운다. 크기는 쓰지 않는다.
	 * 부착 전이거나 TransformComponent가 없으면 출력값을 유지하고 false를 반환한다.
	 */
	bool GetCameraViewInfo(CameraViewInfo& outCameraViewInfo) const;
};
