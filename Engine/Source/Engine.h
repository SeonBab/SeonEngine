#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Object.h"

class FInputState;
class IRenderer;
class UWorld;

/** 게임과 에디터 실행 객체의 공통 계약이다. 창과 Renderer의 소유 및 바깥 반복은 실행 루프가 맡는다. */
class UEngine : public UObject
{
#pragma region Lifecycle

public:
	UEngine();

	/** 남은 World를 파괴한다. 가상 PreExit는 호출하지 않는다. */
	~UEngine() override;

#pragma endregion

#pragma region Execution

public:
	/** 빈 World를 만든다. 기존 World가 있으면 거부한다. 게임 구성과 렌더 상태 생성은 파생 구현에서 수행한다. */
	[[nodiscard]] virtual bool Initialize(IRenderer& renderer);

	/** 초기화와 구분되는 실행 시작 훅이다. 기본 구현은 아무 작업도 하지 않는다. */
	virtual void Start();

	/**
	 * 초기화 성공 후 같은 스레드에서 양수 화면 크기로 한 프레임을 처리한다.
	 * 입력과 Renderer는 호출 동안만 빌린다. 실패하면 false이며 호출자는 반복을 중단하고 정리한다.
	 * true는 그리기 요청 성공이며 GPU 완료나 화면 표시를 보장하지 않는다.
	 */
	[[nodiscard]] virtual bool Tick(float deltaSeconds, const FInputState& input, IRenderer& renderer, int width, int height) = 0;

	/** Renderer 종료 전에 실행 상태를 정리한다. 초기화 전, 부분 실패 후, 반복 호출을 허용한다. */
	virtual void PreExit() = 0;

protected:
	/** 공통 기반이 단독 소유한다. 파생 구현은 자신의 자원을 정리한 뒤 이 World를 해제한다. */
	TUniquePtr<UWorld> world;

#pragma endregion
};
