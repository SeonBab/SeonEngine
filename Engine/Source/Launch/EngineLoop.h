#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Input/InputState.h"
#include "Renderer/Renderer.h"
#include "Windows/WindowsApplication.h"

class UWorld;
class FGameClient;

/** 엔진의 초기화, 프레임 반복, 종료를 조율할 실행 객체다 */
class FEngineLoop
{
public:
	FEngineLoop();
	~FEngineLoop();

	FEngineLoop(const FEngineLoop&) = delete;
	FEngineLoop& operator=(const FEngineLoop&) = delete;
	FEngineLoop(FEngineLoop&&) = delete;
	FEngineLoop& operator=(FEngineLoop&&) = delete;

	/** 초기화, 프레임 반복, 정리를 수행하고 종료 코드를 반환한다 */
	[[nodiscard]] int Run();

private:
	[[nodiscard]] bool Initialize();
	/** 초기화 성공 후 한 프레임을 처리한다. 크기 0은 성공으로 건너뛰고 처리 실패는 false를 반환한다. */
	[[nodiscard]] bool Tick(float deltaSeconds);
	[[nodiscard]] bool Shutdown();

private:
	FInputState input;
	FWindowsApplication application;
	FRenderer renderer;
	TUniquePtr<UWorld> world;
	TUniquePtr<FGameClient> gameClient;
};
