#pragma once

#include "Camera/CameraViewInfo.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Vector3.h"

class FRenderer;
class FScene;

////////////////////////////////////////////////////////////////////////////////
// 게임 액터와 독립적인 편집 카메라의 월드 위치와 도 단위 회전을 보관한다.
////////////////////////////////////////////////////////////////////////////////
class FViewportCameraTransform
{
public:
	/** 보관한 월드 위치의 사본을 반환한다. */
	FVector3 GetLocation() const;

	/** 유한한 월드 위치를 그대로 저장한다. 액터나 렌더 상태를 갱신하지 않는다. */
	void SetLocation(FVector3 newLocation);

	/** 보관한 도 단위 각도의 사본을 반환한다. 대표 각도 변환은 하지 않는다. */
	FRotator GetRotation() const;

	/** 유한한 도 단위 각도를 그대로 저장한다. pitch 제한이나 각도 정규화는 입력 처리의 별도 정책이다. */
	void SetRotation(FRotator newRotation);

private:
	FVector3 viewLocation{};
	FRotator viewRotation{}; // Quat으로 왕복하지 않아 조작 중인 각도 표현을 유지한다.
};

////////////////////////////////////////////////////////////////////////////////
// 원근과 직교 편집 시점, 투영 설정을 보관하고 선택한 시점으로 장면을 그린다.
////////////////////////////////////////////////////////////////////////////////
class FEditorViewportClient
{
public:
	/** 현재 투영 방식을 값으로 반환한다. */
	ECameraProjectionMode GetProjectionMode() const;

	/** Perspective 또는 Orthographic을 저장한다. 각 시점의 위치와 회전을 유지하며 화면 다시 그리기는 요청하지 않는다. */
	void SetProjectionMode(ECameraProjectionMode newProjectionMode);

	/** 보관한 원근 투영의 수평 시야각을 도 단위로 반환한다. */
	float GetHorizontalFovDegrees() const;

	/** 원근 투영의 수평 시야각을 도 단위로 저장한다. 유효 범위는 0도 초과 180도 미만이며 값 검사나 보정은 하지 않는다. */
	void SetHorizontalFovDegrees(float newHorizontalFovDegrees);

	/** 보관한 직교 투영의 가로 폭을 월드 단위로 반환한다. */
	float GetOrthographicWidth() const;

	/** 직교 투영의 가로 폭을 월드 단위로 저장한다. 유효한 투영에는 유한한 양수가 필요하며 값 검사나 보정은 하지 않는다. */
	void SetOrthographicWidth(float newOrthographicWidth);

	/** 보관한 가까운 클리핑 거리를 월드 단위로 반환한다. */
	float GetNearClipDistance() const;

	/** 가까운 클리핑 거리를 월드 단위로 저장한다. 유효한 투영에는 유한한 0 < near < far가 필요하며 값 검사나 보정은 하지 않는다. */
	void SetNearClipDistance(float newNearClipDistance);

	/** 보관한 먼 클리핑 거리를 월드 단위로 반환한다. */
	float GetFarClipDistance() const;

	/** 먼 클리핑 거리를 월드 단위로 저장한다. 유효한 투영에는 유한한 0 < near < far가 필요하며 값 검사나 보정은 하지 않는다. */
	void SetFarClipDistance(float newFarClipDistance);

	/** 현재 투영 방식의 편집 시점을 반환한다. 참조는 이 클라이언트가 살아 있는 동안 유효하며 모드가 바뀌어도 원래 선택한 시점을 가리킨다. */
	FViewportCameraTransform& GetViewTransform();

	/** 현재 투영 방식의 편집 시점을 읽기 전용으로 반환한다. 참조의 수명과 모드 변경 시 동작은 비const 오버로드와 같다. */
	const FViewportCameraTransform& GetViewTransform() const;

	/** 선택한 편집 시점과 투영 설정의 사본을 반환한다. 반환값은 이후 편집 상태 변경을 추적하지 않으며 회전 각도는 그대로 복사한다. */
	FCameraViewInfo GetCameraView() const;

	/**
	 * 편집 시점 행렬을 전달하고 장면을 그린다. 양수 크기와 유효한 시점 설정이 필요하며 크기는 설정한 viewport와 일치해야 한다.
	 * 호출자는 Renderer 초기화, 장면 렌더 상태 준비, 출력과 viewport 설정 및 BeginFrame을 완료하고 프레임 종료를 담당한다.
	 * Scene과 Renderer는 호출 동안만 빌린다. 월드 갱신이나 프레임 시작/종료는 하지 않으며 실패하면 false다.
	 */
	[[nodiscard]] bool Draw(FScene& scene, FRenderer& renderer, int width, int height) const;

private:
	// 모드를 전환해도 각 모드의 위치와 회전을 유지할 수 있도록 별도로 보관한다.
	FViewportCameraTransform viewTransformPerspective;
	FViewportCameraTransform viewTransformOrthographic;
	ECameraProjectionMode projectionMode = ECameraProjectionMode::Perspective;

	float horizontalFovDegrees = 90.0f; // 원근 투영의 수평 시야각이며 단위는 도다.
	float orthographicWidth = 4.0f;     // 직교 투영의 가로 폭이며 월드 위치와 같은 단위다. Unreal OrthoZoom 수치와는 다르다.
	// 최종 시점의 near/far 거리다. FCameraViewInfo와 같은 기본값이며 0 < near < far를 전제로 사용한다.
	float nearClipDistance = 0.1f;
	float farClipDistance = 1000.0f;
};
