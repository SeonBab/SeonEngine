#include "EngineLoop.h"

#include "Core/Time/FrameTimer.h"
#include "EditorEngine.h"
#include "GameEngine.h"

#include <utility>

#pragma region Lifecycle

FEngineLoop::FEngineLoop()
	: application(input)
{
}

FEngineLoop::~FEngineLoop()
{
	(void)Exit();
}

#pragma endregion

#pragma region Execution

int FEngineLoop::Run(EEngineMode engineMode)
{
	int exitCode = 1;

	if (Initialize(engineMode))
	{
		engine->Start();
		exitCode = 0;

		FFrameTimer frameTimer;
		while (application.PumpMessages(exitCode))
		{
			// 최소화 중에도 기준 시각을 갱신해 복원 시 정지 시간 전체를 전달하지 않는다.
			const float deltaSeconds = frameTimer.Tick();

			if (!Tick(deltaSeconds))
			{
				exitCode = 1;
				break;
			}
		}
	}

	if (!Exit())
	{
		exitCode = 1;
	}

	return exitCode;
}

bool FEngineLoop::Initialize(EEngineMode engineMode)
{
	if (engineMode != EEngineMode::Editor && engineMode != EEngineMode::Game) { return false; }

	const FWindowDescription description{};
	// NOTE: application이 입력을 빌리므로 input을 먼저 생성해 더 오래 유지한다.
	// 단일 창 기준의 실행부 소유이며 여러 창이나 플레이어별 입력이 필요하면 소유와 분배 구조를 재검토한다.
	if (!application.Initialize(description))
	{
		return false;
	}

	int clientWidth = 0;
	int clientHeight = 0;
	if (!application.GetClientSize(clientWidth, clientHeight))
	{
		return false;
	}

	if (!renderer.Initialize(application.GetWindowHandle(), clientWidth, clientHeight))
	{
		application.Shutdown();
		return false;
	}

	if (engineMode == EEngineMode::Editor)
	{
		TUniquePtr<UEditorEngine> editorEngine = MakeUnique<UEditorEngine>();
		UEditorEngine& editor = *editorEngine; // 소유권 이동 뒤에도 같은 객체를 초기화하기 위한 빌린 참조다.
		engine = std::move(editorEngine);

		// 초기화 실패도 Exit -> PreExit에서 정리하도록 호출 전에 소유권을 옮긴다.
		if (!editor.Initialize(renderer) || !editor.InitializeEditorUI(application.GetWindowHandle()))
		{
			return false;
		}

		// Editor 객체를 빌려 연결하며 Exit에서 UI 정리 전에 콜백을 해제한다.
		application.SetMessageHandler([&editor](HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
		{
			return editor.HandleWindowMessage(windowHandle, message, wParam, lParam, outResult);
		});

		return true;
	}
	else
	{
		engine = MakeUnique<UGameEngine>();
	}

	return engine->Initialize(renderer);
}

bool FEngineLoop::Tick(float deltaSeconds)
{
	int clientWidth = 0;
	int clientHeight = 0;

	if (!application.GetClientSize(clientWidth, clientHeight))
	{
		return false;
	}
	// TODO(seon): 최소화, 가려짐 시 CPU 사용을 확인하고 대기, 복원 정책을 적용한다.
	if (clientWidth == 0 || clientHeight == 0)
	{
		return true;
	}

	return engine->Tick(deltaSeconds, input, renderer, clientWidth, clientHeight);
}

bool FEngineLoop::Exit()
{
	// 캡처한 엔진 객체와 그 UI가 정리되기 전에 메시지 연결을 끊는다.
	application.ClearMessageHandler();

	if (engine)
	{
		engine->PreExit();
		engine.reset();
	}
	renderer.Shutdown();

	bool bSuccess = true;
	if (!application.Shutdown())
	{
		bSuccess = false;
	}

	return bSuccess;
}

#pragma endregion
