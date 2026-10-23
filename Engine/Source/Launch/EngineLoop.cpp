#include "EngineLoop.h"

#include "Core/Time/FrameTimer.h"
#include "EditorEngine.h"
#include "GameEngine.h"

FEngineLoop::FEngineLoop()
	: application(input)
{
}

FEngineLoop::~FEngineLoop()
{
	(void)Exit();
}

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
		engine = MakeUnique<UEditorEngine>();
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
