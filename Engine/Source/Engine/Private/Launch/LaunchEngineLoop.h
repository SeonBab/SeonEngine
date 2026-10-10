#pragma once

#include "Engine/EngineLoop.h"
#include "Templates/UniquePtr.h"

#include <chrono>

class IGameProject;
class UEngine;

////////////////////////////////////////////////////////////////////////////////
// 게임 엔진을 소유하고 준비 / 시작 / 명시적 종료를 연결한다.
// EngineMain이 준비 / 시간 Tick / 종료를 호출한다. 창 / GPU 구성 이전은 후속이다.
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

	/** 게임 구현과 엔진을 준비하고 월드 초기 구성 뒤 Start를 호출한다. 중복 / 빈 게임 구현 / 월드 부재 / 구성 실패는 Fatal이며 정상 반환은 0이다. */
	int32 Init() override;

	/** 이전 측정부터의 경과 시간을 게임 엔진에 전달한다. 준비 전 / 종료 후는 생략한다. */
	void Tick();

	/** 엔진 PreExit / 파괴 뒤 게임 프로젝트를 파괴한다. 준비 전 / 반복 종료를 허용한다. */
	void Exit();

	/** 소유 엔진을 비소유 반환한다. 준비 전 / 종료 후에는 nullptr다. */
	UEngine* GetEngine() const { return engine.get(); }

private:
	// 엔진 / 월드 정리가 끝날 때까지 게임 구현을 유지한다.
	TUniquePtr<IGameProject> gameProject;
	TUniquePtr<UEngine> engine;
	/** Init 완료와 각 Tick에서 저장하는 다음 간격 측정의 기준 시각이다. */
	std::chrono::steady_clock::time_point previousTime{};
};
