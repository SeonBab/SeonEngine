#pragma once

#include "WindowsApplication.h"
#include "World.h"

/**
 * 창과 World를 소유하고 프레임 순서(메시지 처리, 입력, 갱신, 렌더 데이터 수집)를 실행한다.
 * 메인 스레드에서 생성, 초기화, 실행, 소멸한다. 게임은 Run 전에 GetWorld로 World를 꾸민다.
 */
class Engine
{
	// 생성과 소멸
public:
	Engine() = default;

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

	/** 창을 만든다. 실패하거나 이미 초기화했으면 false를 반환한다. */
	bool Initialize();

	// 실행
public:
	/** 초기화 성공 후 Run 전에 게임이 구성할 World를 빌려준다. 엔진 수명 동안 유효하다. */
	World& GetWorld();

	/**
	 * 창이 닫힐 때까지 프레임을 반복하고 종료 코드를 반환한다.
	 * Initialize 성공 후 한 번만 호출하며, 그 밖의 호출은 바로 1을 반환한다.
	 */
	int Run();

private:
	/** 한 프레임을 처리한다. deltaTime는 초 단위다. */
	void Tick(float deltaTime);

private:
	// 선언 역순으로 파괴되므로 창을 가장 먼저 선언해 가장 늦게 정리한다.
	WindowsApplication windowsApplication;
	World world;

	// Initialize 성공 시 true, Run을 시작하면 false다. 창 없이 실행하거나 종료 후 다시 실행하는 것을 막는다.
	bool canRun = false;
};
