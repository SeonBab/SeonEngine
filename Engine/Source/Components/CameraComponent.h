#pragma once

#include "ActorComponent.h"
#include "Camera/CameraViewInfo.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Transform.h"

////////////////////////////////////////////////////////////////////////////////
// 카메라 투영 설정을 보관하고 소속 액터의 월드 자세와 함께 시점 사본을 제공한다.
////////////////////////////////////////////////////////////////////////////////
class UCameraComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성만으로 액터에 등록하지 않는다. */
	explicit UCameraComponent(AActor& owner);

	/** 저장된 투영 방식을 반환한다. */
	ECameraProjectionMode GetProjectionMode() const;

	/** 투영 방식을 저장한다. 값 검사는 하지 않는다. */
	void SetProjectionMode(ECameraProjectionMode newProjectionMode);

	/** 가로 시야각을 도 단위로 반환한다. */
	float GetHorizontalFovDegrees() const;

	/** 가로 시야각을 도 단위로 저장한다. 값 검사는 하지 않는다. */
	void SetHorizontalFovDegrees(float newHorizontalFovDegrees);

	/** 직교 투영의 가로 폭을 월드 단위로 반환한다. */
	float GetOrthographicWidth() const;

	/** 직교 투영의 가로 폭을 월드 단위로 저장한다. 값 검사는 하지 않는다. */
	void SetOrthographicWidth(float newOrthographicWidth);

	/** 가까운 클리핑 거리를 월드 단위로 반환한다. */
	float GetNearClipDistance() const;

	/** 가까운 클리핑 거리를 월드 단위로 저장한다. 값 검사는 하지 않는다. */
	void SetNearClipDistance(float newNearClipDistance);

	/** 먼 클리핑 거리를 월드 단위로 반환한다. */
	float GetFarClipDistance() const;

	/** 먼 클리핑 거리를 월드 단위로 저장한다. 값 검사는 하지 않는다. */
	void SetFarClipDistance(float newFarClipDistance);

	/** 소속 액터의 월드 위치, 회전을 반환한다. 크기 배율은 1이며 원본은 변경하지 않는다. */
	[[nodiscard]] FTransform GetCameraWorldTransform() const;

	/** 현재 월드 위치·회전과 투영 설정을 복사해 반환한다. */
	[[nodiscard]] FCameraViewInfo GetCameraView() const;

private:
	ECameraProjectionMode projectionMode = ECameraProjectionMode::Perspective;
	float horizontalFovDegrees = 90.0f;
	float orthographicWidth = 4.0f;
	float nearClipDistance = 0.1f;
	float farClipDistance = 1000.0f;
};
