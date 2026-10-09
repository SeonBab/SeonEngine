#pragma once

#include "Engine/EngineLoop.h"
#include "Templates/UniquePtr.h"

class UEngine;

////////////////////////////////////////////////////////////////////////////////
// 게임 엔진을 소유하고 준비 / 시작 / 명시적 종료를 연결한다.
// 실제 진입점·프레임·창 / GPU 연결은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FEngineLoop : public IEngineLoop
{
public:
	FEngineLoop();
	/** 소유 객체만 파괴한다. Exit / PreExit는 자동 호출하지 않는다. */
	~FEngineLoop() override;

	// 엔진이 이 루프의 주소를 저장하므로 루프 객체 자체의 복사 / 이동은 금지한다.
	FEngineLoop(const FEngineLoop&) = delete;
	FEngineLoop& operator=(const FEngineLoop&) = delete;
	FEngineLoop(FEngineLoop&&) = delete;
	FEngineLoop& operator=(FEngineLoop&&) = delete;

	/** 게임 엔진의 Init / Start를 호출한다. 기존 엔진이 있으면 Fatal이고 정상 반환은 0이다. */
	int32 Init() override;

	/** 엔진 PreExit 후 객체를 파괴한다. 준비 전 / 반복 종료를 허용한다. */
	void Exit();

	/** 소유 엔진을 비소유 반환한다. 준비 전 / 종료 후에는 nullptr다. */
	UEngine* GetEngine() const { return engine.get(); }

private:
	TUniquePtr<UEngine> engine;
};
