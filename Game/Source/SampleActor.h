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

	/** 경과 시간에 따라 로컬 Y축을 좌우 65도 범위에서 왕복 회전한다. */
	void Tick(float deltaSeconds) override;

private:
	float elapsedSeconds = 0.0f;
};
