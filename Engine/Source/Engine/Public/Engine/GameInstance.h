#pragma once

#include "UObject/Object.h"

class FWorld;
struct FWorldContext;
class UEngine;

////////////////////////////////////////////////////////////////////////////////
// 게임 실행 한 번의 상태와 초기화 / 종료 훅을 제공한다.
// 소속 엔진과 컨텍스트는 비소유이며 월드는 엔진의 컨텍스트가 소유한다.
////////////////////////////////////////////////////////////////////////////////
class UGameInstance : public UObject
{
	// 컨텍스트 제거 전에 비소유 세션 연결을 함께 해제한다.
	friend class UEngine;

public:
	/** 유효한 엔진을 소속으로 연결한다. 생성자에서는 Init이나 월드 준비를 하지 않는다. */
	explicit UGameInstance(UEngine& inEngine);

	/** 비소유 연결만 해제한다. Shutdown을 자동 호출하거나 엔진 / 월드를 파괴하지 않는다. */
	~UGameInstance() override;

	/** 소속 엔진을 비소유 반환한다. Outer의 타입이 맞지 않으면 Fatal이다. */
	UEngine* GetEngine() const;

	/** 현재 월드를 비소유 반환한다. 준비 전이나 연결 해제 후에는 nullptr다. */
	FWorld* GetWorld() const;

	/**
	 * 컨텍스트와 빈 월드를 준비한 뒤 Init을 호출한다. 한 객체당 한 번만 준비한다.
	 * 중복 준비 / 엔진의 기존 컨텍스트는 Fatal이다. 맵 로딩 / 게임 시작은 하지 않는다.
	 */
	void InitializeStandalone();

	/**
	 * 프로젝트별 세션 초기화 훅이다. 기본은 빈 동작이고 성공 / 실패를 반환하지 않는다.
	 * InitializeStandalone 경로에서는 월드 연결 후 호출된다. GPU / 시작 맵 준비는 보장하지 않는다.
	 */
	virtual void Init();

	/**
	 * 프로젝트별 정리 후 기반을 호출한다. 기반 구현은 비소유 컨텍스트 / 월드 연결을 해제한다.
	 * 월드와 컨텍스트 자체는 엔진이 별도로 파괴한다. 소멸자는 자동 호출하지 않는다.
	 */
	virtual void Shutdown();

private:
	void DetachWorldContext();

	FWorldContext* worldContext = nullptr;
	bool bStandaloneInitialized = false;
};
