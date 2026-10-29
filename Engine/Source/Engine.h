#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"

class FInputState;
class IRenderer;
class UAssetManager;
class UWorld;

/** 게임과 에디터 실행 객체의 공통 계약이다. 창과 Renderer의 소유 및 바깥 반복은 실행 루프가 맡는다. */
class UEngine : public UObject
{
#pragma region Lifecycle

public:
	UEngine();

	/** 정상 종료는 PreExit가 맡는다. 남은 활성 연결을 먼저 해제하고 공통 자원을 파괴하며 가상 PreExit는 호출하지 않는다. */
	~UEngine() override;

#pragma endregion

#pragma region Services

public:
	/**
	 * 공통 자산 서비스를 사용할 수 있는 활성 Engine을 빌린다. 초기화 전과 공통 종료 후에는 nullptr다.
	 * 파생 초기화와 의존 객체의 종료 중에도 조회할 수 있다. World/GameInstance의 정상 실행 상태는 보장하지 않는다.
	 * 같은 실행 스레드에서만 조회하고 사용하며 소유권과 수명을 연장하지 않는다.
	 */
	[[nodiscard]] static UEngine* Get();

	/** 공통 초기화 후부터 서비스 연결 해제 전까지만 호출한다. 반환 참조는 관리자를 소유하거나 수명을 연장하지 않는다. */
	UAssetManager& GetAssetManager() noexcept;

private:
	static UEngine* activeEngine;

#pragma endregion

#pragma region Execution

public:
	/**
	 * 초기화된 Renderer를 빌리는 AssetManager와 빈 World를 준비한 뒤 활성 조회를 연결한다.
	 * 기존 World나 다른 활성 Engine이 있으면 현재 상태를 유지하고 거부한다. 메모리 할당 예외는 복구하지 않는다.
	 * 게임 구성과 렌더 상태 생성은 파생 구현에서 수행한다. 이 Engine과 자산의 정리 후 Renderer를 종료한다.
	 */
	[[nodiscard]] virtual bool Initialize(IRenderer& renderer);

	/** 초기화와 구분되는 실행 시작 훅이다. 기본 구현은 아무 작업도 하지 않는다. */
	virtual void Start();

	/**
	 * 초기화 성공 후 같은 스레드에서 양수 화면 크기로 한 프레임을 처리한다.
	 * 입력과 Renderer는 호출 동안만 빌린다. 실패하면 false이며 호출자는 반복을 중단하고 정리한다.
	 * true는 그리기 요청 성공이며 GPU 완료나 화면 표시를 보장하지 않는다.
	 */
	[[nodiscard]] virtual bool Tick(float deltaSeconds, const FInputState& input, IRenderer& renderer, int width, int height) = 0;

	/**
	 * 의존 객체 정리 후 파생 구현이 호출한다. World 정리 동안 서비스 조회를 유지하고 연결 해제 후 관리자를 파괴한다.
	 * Renderer가 살아 있는 같은 스레드의 프레임 밖에서 호출한다. 초기화 전·부분 실패·반복 호출은 허용하며 종료 콜백에서 재호출하지 않는다.
	 */
	virtual void PreExit();

private:
	/** 공통 기반이 단독 소유한다. World보다 먼저 선언해 World 정리 후 캐시를 해제한다. */
	TUniquePtr<UAssetManager> assetManager;

protected:
	/**
	 * 공통 초기화된 이 Engine의 World에 논리 경로의 JSON 맵을 복원한다. 파생 초기화에서 플레이 전에 한 번 호출한다.
	 * 기본 타입과 게임 제공 타입을 지역 등록표에 준비하며 GameInstance·카메라·UI·렌더 상태·플레이는 시작하지 않는다.
	 * 경로/등록/로딩 실패는 false다. 부분 구성과 자산 캐시가 남을 수 있으므로 파생 초기화가 PreExit로 정리한다.
	 * 외부 라이브러리·할당 예외는 복구하지 않는다. Renderer는 World와 공통 서비스 정리 뒤 종료해야 한다.
	 */
	[[nodiscard]] bool LoadMap(FStringView contentPath);

	/** 공통 기반이 단독 소유한다. 파생 구현은 자신의 자원을 정리한 뒤 이 World를 해제한다. */
	TUniquePtr<UWorld> world;

#pragma endregion
};
