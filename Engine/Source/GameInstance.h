#pragma once

#include "Object.h"

class FGameClient;
class FRenderer;
class UWorld;

////////////////////////////////////////////////////////////////////////////////
// 게임 실행별 초기화 훅을 제공한다. World와 Client는 소유하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UGameInstance : public UObject
{
public:
	/** 기반 포인터로 파생 게임 실행 객체를 파괴한다. 초기화나 별도 종료 훅을 자동 호출하지 않는다. */
	~UGameInstance() override;

	/**
	 * 호출자가 World와 Client를 생성한 뒤 렌더 상태 준비와 BeginPlay 전에 한 번 호출한다. 중복과 재진입은 검사하지 않는다.
	 * 초기화된 Renderer와 나머지 인수는 호출 동안만 빌리며 소유권을 넘겨받거나 참조를 저장하지 않는다.
	 * 기본 구현은 아무 작업 없이 true를 반환한다. false이면 호출자는 플레이를 시작하지 않고 부분 생성 객체를 정리한다.
	 * 실패한 초기화를 자동 롤백하지 않는다.
	 */
	[[nodiscard]] virtual bool Initialize(UWorld& world, FGameClient& gameClient, FRenderer& renderer);
};
