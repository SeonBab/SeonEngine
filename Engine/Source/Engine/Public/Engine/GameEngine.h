#pragma once

#include "Engine/Engine.h"
#include "Templates/UniquePtr.h"

class UGameInstance;

////////////////////////////////////////////////////////////////////////////////
// 게임 세션을 소유하고 준비 / 명시적 종료를 연결하는 게임용 엔진이다.
// 창 / 렌더러 / 월드 프레임 갱신은 아직 연결하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UGameEngine : public UEngine
{
public:
	UGameEngine();
	/** 소유 객체를 파괴한다. PreExit나 세션 Shutdown을 자동 호출하지 않는다. */
	~UGameEngine() override;

	/** 기반 루프 연결 후 기본 세션과 빈 월드를 준비한다. 세션이 이미 있으면 Fatal이다. */
	void Init(IEngineLoop* inEngineLoop) override;

	/** 현재는 빈 동작이다. 월드 갱신 / 렌더링 / 유휴 정책은 후속 연결이다. */
	void Tick(float deltaSeconds, bool bIdleMode) override;

	/** 세션 Shutdown 후 월드 / 컨텍스트와 세션 객체를 정리한다. 준비 전 / 반복 종료도 허용한다. */
	void PreExit() override;

	/** 소유한 세션을 비소유 반환한다. 준비 전 / 종료 후에는 nullptr다. */
	UGameInstance* GetGameInstance() const { return gameInstance.get(); }

private:
	TUniquePtr<UGameInstance> gameInstance;
};
