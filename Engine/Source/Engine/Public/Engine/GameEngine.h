#pragma once

#include "Engine/Engine.h"
#include "Engine/GameInstanceFactory.h"
#include "Templates/UniquePtr.h"

class UGameInstance;

////////////////////////////////////////////////////////////////////////////////
// 게임 세션을 소유하고 준비 / 시작 / 명시적 종료를 연결하는 게임용 엔진이다.
// 실행 루프가 Init / Start / Tick / PreExit를 호출한다. 창과 렌더러는 실행부가 관리한다.
////////////////////////////////////////////////////////////////////////////////
class UGameEngine : public UEngine
{
public:
	UGameEngine();
	/** 소유 객체를 파괴한다. PreExit나 세션 Shutdown을 자동 호출하지 않는다. */
	~UGameEngine() override;

	/**
	 * 생성 함수 주소만 등록한다. 빈 입력 / 이미 등록됨은 false이며 기존 값을 유지한다.
	 * 게임 모듈 준비에서 엔진 초기화 전에 호출한다. 시점 검사 / 세션 생성은 하지 않는다.
	 */
	[[nodiscard]] static bool RegisterGameInstanceFactory(FGameInstanceFactory inFactory);

	/**
	 * 등록 주소와 같은 유효한 주소만 해제한다. 빈 입력 / 주소 불일치는 false와 기존 값 유지다.
	 * 성공은 true이며 이미 생성된 세션은 파괴하지 않는다. 세션 정리 뒤 모듈 종료에서 호출한다.
	 */
	[[nodiscard]] static bool UnregisterGameInstanceFactory(FGameInstanceFactory inFactory);

	/** 기반 루프 연결 후 등록된 생성 함수 또는 기본 세션으로 세션과 월드를 준비한다. 기존 세션 / 빈 생성 결과는 Fatal이다. */
	void Init(IEngineLoop* inEngineLoop) override;

	/** 준비된 세션의 시작 훅을 호출한다. 세션 부재는 Fatal이며 초기화 후 한 번 호출해야 한다. */
	void Start() override;

	/** 현재 세션 월드의 Tick 완료 뒤 제거 보관 액터를 실제 파괴한다. 렌더링은 실행부가 별도로 진행한다. */
	void Tick(float deltaSeconds) override;

	/** 현재 게임 세션의 월드를 비소유 반환한다. 준비 전 / 세션 연결 해제 / 종료 후에는 nullptr다. */
	FWorld* GetWorld() const override;

	/** 세션 Shutdown 후 월드 / 컨텍스트와 세션 객체를 정리한다. 준비 전 / 반복 종료도 허용한다. */
	void PreExit() override;

	/** 소유한 세션을 비소유 반환한다. 준비 전 / 종료 후에는 nullptr다. */
	UGameInstance* GetGameInstance() const { return gameInstance.get(); }

private:
	// 엔진 생성 전 등록하는 공통 함수 주소다. Init에서 세션 생성에 사용한다.
	static FGameInstanceFactory gameInstanceFactory;

	TUniquePtr<UGameInstance> gameInstance;
};
