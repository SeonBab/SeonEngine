#pragma once

#include "Actor.h"

////////////////////////////////////////////////////////////////////////////////
// 게임 Tick에서 기본 Transform을 회전시키는 최소 샘플 액터다.
// 메시 구성은 게임 구성 함수, 렌더 변환 전달은 엔진이 담당한다.
////////////////////////////////////////////////////////////////////////////////
class ASampleActor : public AActor
{
public:
	/** 월드를 연결하고 기반 액터의 기본 Transform을 준비한다. */
	explicit ASampleActor(UWorld& world);

	/** 초 단위 경과 시간에 맞춰 로컬 Z축 회전을 초당 45도 증가시킨다. */
	void Tick(float deltaSeconds) override;
};
