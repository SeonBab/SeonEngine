#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Engine.h"

class FGameClient;
class UGameInstance;

/** 게임 실행 객체와 로컬 Client를 단독 소유하고 공통 World의 게임 갱신과 그리기를 연결한다. */
class UGameEngine : public UEngine
{
#pragma region Lifecycle

public:
	UGameEngine();

	/** PreExit 누락에 대비해 파생 멤버가 살아 있을 때 정리한다. Renderer와 창은 소유하지 않는다. */
	~UGameEngine() override;

	UGameEngine(const UGameEngine&) = delete;

	UGameEngine& operator=(const UGameEngine&) = delete;

	UGameEngine(UGameEngine&&) = delete;

	UGameEngine& operator=(UGameEngine&&) = delete;

#pragma endregion

#pragma region Execution

public:
	/** 공통 서비스와 gameDefaultMap을 준비한 뒤 게임 제공 CreateGameInstance로 실행 객체를 만든다. World와 Client의 게임 구성 뒤 렌더 상태를 준비한다. 중복 초기화는 기존 상태를 보존하고 거부하며 새 초기화의 bool 실패는 PreExit로 정리한다. 플레이는 Start에서 시작하며 종료 콜백에서 초기화하지 않는다. */
	[[nodiscard]] bool Initialize(IRenderer& renderer) override;

	/** 초기화 성공 후 프레임 밖에서 월드 플레이를 시작한다. 반복 호출은 월드의 시작 상태로 무시한다. */
	void Start() override;

	/** 초기화와 Start 완료 상태 및 양수 화면 크기가 필요하다. 입력·월드·렌더 상태 갱신 뒤 프레임을 시작하고 자동 캡처, 기본 장면, UI 순서로 그린다. 실패하면 반복을 중단하고 정리한다. */
	[[nodiscard]] bool Tick(float deltaSeconds, const FInputState& input, IRenderer& renderer, int width, int height) override;

	/** 프레임 밖에서 플레이 종료 → GameInstance → Client → 기반 World/서비스 순서로 정리한다. 초기화 실패와 반복 호출을 허용하며 종료 콜백에서 재호출하지 않는다. */
	void PreExit() override;

private:
	TUniquePtr<FGameClient> gameClient;

	TUniquePtr<UGameInstance> gameInstance; // Client/World보다 먼저 파괴한다.

	bool bInitialized = false; // 게임 준비가 완료됐다. Tick에는 Start 완료도 필요하다.

#pragma endregion
};
