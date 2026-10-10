#pragma once

#include "Engine/Engine.h"
#include "Templates/UniquePtr.h"

class UGameInstance;

////////////////////////////////////////////////////////////////////////////////
// 게임 세션을 소유하고 준비 / 명시적 종료를 연결하는 게임용 엔진이다.
// 월드 시간 갱신을 전달한다. 실제 루프 / 창 / 렌더러 연결은 후속이다.
////////////////////////////////////////////////////////////////////////////////
class UGameEngine : public UEngine
{
public:
	UGameEngine();
	/** 소유 객체를 파괴한다. PreExit나 세션 Shutdown을 자동 호출하지 않는다. */
	~UGameEngine() override;

	/** 기반 루프 연결 후 기본 세션과 빈 월드를 준비한다. 세션이 이미 있으면 Fatal이다. */
	void Init(IEngineLoop* inEngineLoop) override;

	/** 현재 세션 월드의 Tick 완료 뒤 제거 보관 액터를 실제 파괴한다. 렌더링은 후속이다. */
	void Tick(float deltaSeconds) override;

	/** 현재 게임 세션의 월드를 비소유 반환한다. 준비 전 / 세션 연결 해제 / 종료 후에는 nullptr다. */
	FWorld* GetWorld() const override;

	/** 세션 Shutdown 후 월드 / 컨텍스트와 세션 객체를 정리한다. 준비 전 / 반복 종료도 허용한다. */
	void PreExit() override;

	/** 소유한 세션을 비소유 반환한다. 준비 전 / 종료 후에는 nullptr다. */
	UGameInstance* GetGameInstance() const { return gameInstance.get(); }

private:
	TUniquePtr<UGameInstance> gameInstance;
};
