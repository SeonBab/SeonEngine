#pragma once

#include "Templates/UniquePtr.h"
#include "UObject/Object.h"

class FWorld;
struct FWorldContext;
class IEngineLoop;

////////////////////////////////////////////////////////////////////////////////
// 게임과 에디터 엔진의 공통 실행 기반이다. 실제 프레임 동작은 파생 엔진이 제공한다.
// 루프 연결은 비소유이며 객체 생성과 실행 초기화를 구분한다.
////////////////////////////////////////////////////////////////////////////////
class UEngine : public UObject
{
public:
	/** 소속 없는 엔진 객체를 만든다. 실행 준비는 Init에서 한다. */
	UEngine();
	/** 잔여 컨텍스트 연결 / 소유 월드를 정리한다. PreExit나 세션 Shutdown은 자동 호출하지 않는다. */
	~UEngine() override;

	/**
	 * 공통 루프 연결을 저장한다. 파생 구현은 기반 Init을 먼저 호출한다.
	 * @param inEngineLoop nullptr는 Fatal로 처리한다. 엔진이 사용하는 동안 루프가 살아 있어야 한다.
	 */
	virtual void Init(IEngineLoop* inEngineLoop);

	/**
	 * Init 이후 게임 실행을 시작하는 통로다. 현재 기반 구현은 비어 있다.
	 * 생성자나 Init에서 자동 호출하지 않는다.
	 */
	virtual void Start();

	/**
	 * 한 프레임의 실행 동작을 파생 엔진이 제공한다.
	 * @param deltaSeconds 루프가 측정한 경과 시간(초). 시간 측정은 엔진 밖의 책임이다.
	 */
	virtual void Tick(float deltaSeconds) = 0;

	/** 파생 엔진이 선택한 현재 월드를 비소유 반환한다. 기반은 nullptr이며 대상의 수명을 연장하지 않는다. */
	virtual FWorld* GetWorld() const;

	/**
	 * 객체 파괴 전 실행 상태를 정리하는 통로다. 현재 기반 구현은 비어 있다.
	 * 파생 구현은 자신이 필요한 정리를 마친 뒤 기반 PreExit를 호출한다.
	 * 소멸자가 자동 호출하지 않으며 루프 객체를 파괴하지 않는다.
	 */
	virtual void PreExit();

	/**
	 * 내부 실행용 컨텍스트 하나를 생성해 소유한다. 이미 존재하면 Fatal이다.
	 * 반환 참조는 컨텍스트 제거 시 무효다. 일반 게임 코드는 직접 관리하지 않는다.
	 */
	FWorldContext& CreateNewWorldContext();

	/**
	 * 해당 월드의 비소유 연결을 끊고 컨텍스트 / 월드를 파괴한다.
	 * 일치하는 컨텍스트가 없으면 변경하지 않는다. Shutdown은 자동 호출하지 않는다.
	 */
	virtual void DestroyWorldContext(FWorld* inWorld);

private:
	IEngineLoop* engineLoop = nullptr;
	TUniquePtr<FWorldContext> worldContext;
};
