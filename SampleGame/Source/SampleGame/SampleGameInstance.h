#pragma once

#include "Engine/GameInstance.h"
#include "Engine/GameInstanceFactory.h"

////////////////////////////////////////////////////////////////////////////////
// SampleGame의 게임 세션이다. 시작 훅에서 테스트 삼각형을 배치한다.
////////////////////////////////////////////////////////////////////////////////
class USampleGameInstance : public UGameInstance
{
public:
	/** 소속 엔진을 기반 세션에 전달한다. 생성 중에 세션 준비 / 시작은 하지 않는다. */
	explicit USampleGameInstance(UEngine& inEngine);

	/** 준비된 월드에 테스트 삼각형을 배치한다. 월드 부재 / 메시 추가 실패는 Fatal이다. */
	void StartGameInstance() override;
};

/** 미초기화 게임 세션을 만들어 소유권을 반환한다. 준비 / 시작은 호출자가 담당한다. */
[[nodiscard]] TUniquePtr<UGameInstance> CreateSampleGameInstance(UEngine& inEngine);
