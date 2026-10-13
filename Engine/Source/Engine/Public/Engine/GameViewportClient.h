#pragma once

#include "UObject/Object.h"
#include "ViewportClient.h"

class UEngine;

////////////////////////////////////////////////////////////////////////////////
// 게임용 화면 클라이언트의 최소 기반이다. 키 입력은 아직 부모의 기본 미처리를 사용한다.
// Unreal은 FViewportClient → FCommonViewportClient → FGameplayViewportClient를 거쳐
// UScriptViewportClient에서 UObject와 결합하고 UGameViewportClient에 FExec도 더한다.
// 자체는 위 중간 계층과 콘솔 계약을 생략하고 UObject·FViewportClient를 직접 상속한다.
// 전체 Unreal 계층과 동일하지 않으며, 공통 화면·월드·스크립트 기능 필요 시 계층을 재검토한다.
////////////////////////////////////////////////////////////////////////////////
class UGameViewportClient : public UObject, public FViewportClient
{
public:
	/** 엔진을 비소유 Outer로 연결한다. 생성 시 입력 연결이나 실행 초기화를 수행하지 않는다. */
	explicit UGameViewportClient(UEngine& inEngine);

	/** 기반 포인터를 통한 파괴를 지원한다. 소속 엔진을 파괴하거나 종료 훅을 호출하지 않는다. */
	~UGameViewportClient() override = default;
};
