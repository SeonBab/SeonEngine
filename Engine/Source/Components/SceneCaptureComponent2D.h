#pragma once

#include "ActorComponent.h"
#include "Camera/CameraViewInfo.h"
#include "Core/Templates/SharedPtr.h"

class IRenderer;
class UTextureRenderTarget2D;

////////////////////////////////////////////////////////////////////////////////
// 캡처 시점과 출력 타겟을 보관하고 소속 월드의 장면을 한 번 그리는 순서를 조율한다.
////////////////////////////////////////////////////////////////////////////////
class USceneCaptureComponent2D : public UActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성만으로 액터에 등록하지 않는다. */
	explicit USceneCaptureComponent2D(AActor& owner);

	/**
	 * 캡처 결과를 받을 타겟의 공유 참조를 저장한다. nullptr는 지정을 해제한다.
	 * GPU 자원 생성이나 출력 전환은 수행하지 않는다.
	 */
	void SetTextureTarget(TSharedPtr<UTextureRenderTarget2D> newTextureTarget);

	/** 타겟의 공유 참조를 사본으로 반환한다. 미지정이면 nullptr다. */
	[[nodiscard]] TSharedPtr<UTextureRenderTarget2D> GetTextureTarget() const;

	/** 월드의 매 프레임 자동 캡처 여부를 저장한다. 수동 CaptureScene 호출은 제한하지 않는다. */
	void SetCaptureEveryFrame(bool enabled);

	/** 매 프레임 자동 캡처 설정을 반환한다. Tick 활성 설정과는 독립적이다. */
	[[nodiscard]] bool IsCaptureEveryFrameEnabled() const;

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

	/** 소속 액터의 월드 위치·회전과 저장된 투영 설정을 사본으로 반환한다. */
	[[nodiscard]] FCameraViewInfo GetCaptureView() const;

	/**
	 * 렌더 상태 갱신을 마친 소속 월드의 장면을 같은 스레드의 활성 프레임 안에서 타겟에 한 번 그린다.
	 * 타겟 크기로 종횡비를 계산하며 유효한 투영 설정을 요구한다. 중첩 캡처는 지원하지 않는다.
	 * 성공하면 창 출력과 기존 변환 행렬을 복원한다. UI 그리기와 EndFrame/Present는 수행하지 않는다.
	 * 실패하면 false이며 호출자는 렌더링을 중단하고 Renderer를 Shutdown해야 한다.
	 * 출력 전환 이후 실패하면 타겟 활성 상태가 남을 수 있다. true는 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool CaptureScene(IRenderer& renderer) const;

private:
	// World가 매 프레임 조회한다. false여도 수동 CaptureScene 호출은 가능하다.
	bool bCaptureEveryFrame = true;

	ECameraProjectionMode projectionMode = ECameraProjectionMode::Perspective;
	float horizontalFovDegrees = 90.0f;
	float orthographicWidth = 4.0f;
	float nearClipDistance = 0.1f;
	float farClipDistance = 1000.0f;

	// 캡처 결과를 받을 타겟의 공유 참조다. nullptr는 미지정 상태다.
	TSharedPtr<UTextureRenderTarget2D> textureTarget;
};
