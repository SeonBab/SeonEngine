#pragma once

#include "Camera/CameraViewInfo.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Vector3.h"
#include "Renderer/TextureRenderTarget2D.h"

class IRenderer;
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
// 원근과 직교 편집 시점, 투영 설정과 장면용 렌더 타겟을 보관하고 선택한 시점으로 장면을 그린다.
////////////////////////////////////////////////////////////////////////////////
class FEditorViewportClient
{
#pragma region View

public:
	/** 현재 투영 방식의 편집 시점을 반환한다. 참조는 이 클라이언트가 살아 있는 동안 유효하며 모드가 바뀌어도 원래 선택한 시점을 가리킨다. */
	FViewportCameraTransform& GetViewTransform();

	/** 현재 투영 방식의 편집 시점을 읽기 전용으로 반환한다. 참조의 수명과 모드 변경 시 동작은 비const 오버로드와 같다. */
	const FViewportCameraTransform& GetViewTransform() const;

	/** 선택한 편집 시점과 투영 설정의 사본을 반환한다. 반환값은 이후 편집 상태 변경을 추적하지 않으며 회전 각도는 그대로 복사한다. */
	FCameraViewInfo GetCameraView() const;

private:
	// 모드를 전환해도 각 모드의 위치와 회전을 유지할 수 있도록 별도로 보관한다.
	FViewportCameraTransform viewTransformPerspective;

	FViewportCameraTransform viewTransformOrthographic;

#pragma endregion

#pragma region Projection

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

private:
	ECameraProjectionMode projectionMode = ECameraProjectionMode::Perspective;

	float horizontalFovDegrees = 90.0f; // 원근 투영의 수평 시야각이며 단위는 도다.

	float orthographicWidth = 4.0f; // 직교 투영의 가로 폭이며 월드 위치와 같은 단위다. Unreal OrthoZoom 수치와는 다르다.

	// 최종 시점의 near/far 거리다. FCameraViewInfo와 같은 기본값이며 0 < near < far를 전제로 사용한다.
	float nearClipDistance = 0.1f;

	float farClipDistance = 1000.0f;

#pragma endregion

#pragma region RenderTarget

public:
	/**
	 * 초기화된 동일 Renderer로 프레임 시작 전에 양수 픽셀 크기의 타겟을 준비한다. 같은 크기는 생성 없이 재사용한다.
	 * 성공 시에만 GPU 데이터와 크기를 교체하며 실패하면 기존 타겟을 유지한다. 교체 전 기존 데이터의 비소유 사용을 끝내야 한다.
	 * Renderer와 장치 상태를 별도로 검사하지 않으므로 같은 크기의 성공도 현재 GPU 사용 가능 여부를 보장하지 않는다.
	 */
	[[nodiscard]] bool ResizeRenderTarget(IRenderer& renderer, int32 width, int32 height);

	/** 소유 타겟을 읽기 전용으로 빌린다. 참조는 Client 수명 동안 유효하며 초기에는 GPU 데이터 없이 크기가 0이다. */
	const UTextureRenderTarget2D& GetRenderTarget() const;

	/** UI와 출력의 비소유 사용을 끝낸 뒤 타깃의 모든 렌더 데이터를 해제한다. 크기는 0으로 돌아가며 카메라는 유지한다. 반복 호출을 허용한다. */
	void ReleaseRenderTarget();

private:
	UTextureRenderTarget2D renderTarget; // Client가 GPU 데이터를 소유한다. 기본 생성만으로 GPU 자원을 만들지는 않는다.

#pragma endregion

#pragma region Drawing

public:
	/**
	 * 준비한 소유 타겟의 실제 크기로 편집 시점 행렬을 계산하고 그 타겟에 장면을 그린 뒤 창 출력을 복원한다.
	 * 호출자는 동일 Renderer 초기화, 양수 크기의 타겟·장면 렌더 상태 준비 및 BeginFrame을 완료하고 프레임 종료를 담당한다.
	 * 다른 렌더 타겟이 활성인 상태에서는 호출하지 않는다. 장면 그리기가 실패해도 출력 복원을 시도하며 어느 쪽이든 실패하면 false다.
	 * 실패 후에는 이후 제출·Present를 진행하지 않고 Shutdown으로 정리한다. 복원 실패 시 타겟이 활성 상태일 수 있다.
	 * Scene과 Renderer는 호출 동안만 빌린다. 월드 갱신이나 프레임 시작/종료는 하지 않는다.
	 */
	[[nodiscard]] bool Draw(FScene& scene, IRenderer& renderer) const;

#pragma endregion
};
