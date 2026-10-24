#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"
#include "Input/InputState.h"
#include "Renderer/Renderer.h"
#include "Windows/WindowsApplication.h"

class UEngine;

////////////////////////////////////////////////////////////////////////////////
// 초기화 때 선택하는 실행 모드다. 실행 중 모드 전환은 하지 않는다.
////////////////////////////////////////////////////////////////////////////////
enum class EEngineMode : uint8
{
	Editor,
	Game
};

/** 엔진의 초기화, 프레임 반복, 종료를 조율할 실행 객체다 */
class FEngineLoop
{
#pragma region Lifecycle

public:
	FEngineLoop();

	~FEngineLoop();

	FEngineLoop(const FEngineLoop&) = delete;

	FEngineLoop& operator=(const FEngineLoop&) = delete;

	FEngineLoop(FEngineLoop&&) = delete;

	FEngineLoop& operator=(FEngineLoop&&) = delete;

#pragma endregion

#pragma region Execution

public:
	/** 지정한 실행 모드로 초기화, 프레임 반복, 정리를 수행한다. 기본은 편집이며 잘못된 모드는 실패로 종료한다. */
	[[nodiscard]] int Run(EEngineMode engineMode = EEngineMode::Editor);

private:
	[[nodiscard]] bool Initialize(EEngineMode engineMode);

	/** 초기화 성공 후 한 프레임을 처리한다. 크기 0은 성공으로 건너뛰고 처리 실패는 false를 반환한다. */
	[[nodiscard]] bool Tick(float deltaSeconds);

	[[nodiscard]] bool Exit();

private:
	FInputState input;

	FWindowsApplication application;

	FRenderer renderer;

	TUniquePtr<UEngine> engine;

#pragma endregion
};
