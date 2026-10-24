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

	/** 남은 GameInstance, Client, World 순서로 파괴한다. PreExit를 자동 호출하지 않으며 Renderer와 창은 소유하지 않는다. */
	~UGameEngine() override;

	UGameEngine(const UGameEngine&) = delete;

	UGameEngine& operator=(const UGameEngine&) = delete;

	UGameEngine(UGameEngine&&) = delete;

	UGameEngine& operator=(UGameEngine&&) = delete;

#pragma endregion

#pragma region Execution

public:
	/** 게임 제공 CreateGameInstance로 실행 객체를 만들고 World와 Client 초기화 뒤 렌더 상태를 준비한다. 기존 소유 객체는 거부하고 부분 실패는 PreExit로 정리한다. 종료 콜백에서 호출하지 않는다. */
	[[nodiscard]] bool Initialize(FRenderer& renderer) override;

	/** 초기화 성공 후 프레임 밖에서 월드 플레이를 시작한다. 반복 호출은 월드의 시작 상태로 무시한다. */
	void Start() override;

	/** 초기화와 Start 완료 상태 및 양수 화면 크기가 필요하다. 입력 평가, 월드 갱신, 시점과 렌더 상태 전달, 장면과 UI 그리기를 수행한다. */
	[[nodiscard]] bool Tick(float deltaSeconds, const FInputState& input, FRenderer& renderer, int width, int height) override;

	/** 루프가 프레임 밖에서 World의 플레이를 끝낸 뒤 GameInstance, Client, World를 해제한다. 초기화 실패와 반복 호출을 허용하며 종료 콜백에서 재호출하지 않는다. */
	void PreExit() override;

private:
	TUniquePtr<FGameClient> gameClient;

	TUniquePtr<UGameInstance> gameInstance; // Client/World보다 먼저 파괴한다.

	bool bInitialized = false; // 게임 준비가 완료됐다. Tick에는 Start 완료도 필요하다.

#pragma endregion
};
