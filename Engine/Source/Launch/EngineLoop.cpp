#include "EngineLoop.h"

#include "Components/CameraComponent.h"
#include "Core/Math/Matrix4.h"
#include "Core/Time/FrameTimer.h"
#include "GameClient.h"
#include "GameEntry.h"
#include "Input/InputState.h"
#include "Input/PlayerInput.h"
#include "Renderer/Renderer.h"
#include "Windows/WindowsApplication.h"
#include "World.h"

FEngineLoop::FEngineLoop()
	: application(input)
{
}

FEngineLoop::~FEngineLoop()
{
	(void)Shutdown();
}

int FEngineLoop::Run()
{
	int exitCode = 1;

	if (Initialize())
	{
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

	if (!Shutdown())
	{
		exitCode = 1;
	}

	return exitCode;
}

bool FEngineLoop::Initialize()
{
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

	// 게임 구성 실패와 정상 종료 모두 Client, World, Renderer 순서로 정리한다.
	world = MakeUnique<UWorld>();
	gameClient = MakeUnique<FGameClient>();
	if (!ConfigureGameWorld(*world) || !ConfigureGameClient(*gameClient, renderer))
	{
		return false;
	}

	return world->CreateRenderStates(renderer);
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
	UPlayerInput& playerInput = gameClient->GetPlayerInput();
	if (!playerInput.EvaluateInputActions(gameClient->GetInputMappingContext(), input))
	{
		return false;
	}

	world->Tick(deltaSeconds, input, playerInput);

	FMatrix4 worldToClip;
	worldToClip.SetIdentity();
	if (const UCameraComponent* camera = world->GetActiveCamera())
	{
		const float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
		const FCameraViewInfo view = camera->GetCameraView();
		worldToClip = view.GetWorldToClipMatrix(aspectRatio);
	}

	if (!renderer.SetWorldToClip(worldToClip))
	{
		return false;
	}

	if (!world->SendAllEndOfFrameUpdates(renderer))
	{
		return false;
	}

	if (!renderer.BeginFrame(clientWidth, clientHeight))
	{
		return false;
	}

	if (!world->GetScene().Draw(renderer))
	{
		return false;
	}

	if (!gameClient->GetWidgetSubsystem().Draw(renderer))
	{
		return false;
	}

	if (!renderer.EndFrame())
	{
		return false;
	}

	return true;
}

bool FEngineLoop::Shutdown()
{
	gameClient.reset();
	world.reset();
	renderer.Shutdown();

	bool bSuccess = true;
	if (!application.Shutdown())
	{
		bSuccess = false;
	}

	return bSuccess;
}
