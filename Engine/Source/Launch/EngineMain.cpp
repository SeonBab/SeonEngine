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

int main()
{
	const FWindowDescription description{};
	// NOTE: application이 입력을 빌리므로 input을 먼저 생성해 더 오래 유지한다.
	// 단일 창 기준의 실행부 소유이며 여러 창·플레이어별 입력이 필요하면 소유·분배 구조를 재검토한다.
	// 근거: ProjectSetupDecisions의 "입력 상태 소유 방향 결정 — 2026-10-18".
	FInputState input;
	FWindowsApplication application(input);
	if (!application.Initialize(description))
	{
		return 1;
	}

	int clientWidth = 0;
	int clientHeight = 0;
	if (!application.GetClientSize(clientWidth, clientHeight))
	{
		application.Shutdown();
		return 1;
	}

	FRenderer renderer;
	if (!renderer.Initialize(application.GetWindowHandle(), clientWidth, clientHeight))
	{
		application.Shutdown();
		return 1;
	}

	int exitCode = 0;
	// 게임 구성 실패와 정상 종료 모두 Client → World → Renderer 순서로 정리한다.
	{
		UWorld world;
		FGameClient gameClient;
		if (!ConfigureGameWorld(world) || !ConfigureGameClient(gameClient, renderer) || !world.CreateRenderStates(renderer))
		{
			exitCode = 1;
		}
		else
		{
			FFrameTimer frameTimer;
			while (application.PumpMessages(exitCode))
			{
				// 최소화 중에도 기준 시각을 갱신해 복원 시 정지 시간 전체를 전달하지 않는다.
				const float deltaSeconds = frameTimer.Tick();

				if (!application.GetClientSize(clientWidth, clientHeight))
				{
					exitCode = 1;
					break;
				}
				// TODO(seon): 최소화·가려짐 시 CPU 사용을 확인하고 대기·복원 정책을 적용한다.
				if (clientWidth == 0 || clientHeight == 0)
				{
					continue;
				}
				UPlayerInput& playerInput = gameClient.GetPlayerInput();
				if (!playerInput.EvaluateInputActions(gameClient.GetInputMappingContext(), input))
				{
					exitCode = 1;
					break;
				}

				world.Tick(deltaSeconds, input, playerInput);

				FMatrix4 worldToClip;
				worldToClip.SetIdentity();
				if (const UCameraComponent* camera = world.GetActiveCamera())
				{
					const float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
					const FCameraViewInfo view = camera->GetCameraView();
					worldToClip = view.GetWorldToClipMatrix(aspectRatio);
				}

				if (!renderer.SetWorldToClip(worldToClip) || !world.SendAllEndOfFrameUpdates(renderer) || !renderer.BeginFrame(clientWidth, clientHeight) || !world.GetScene().Draw(renderer) || !gameClient.GetWidgetSubsystem().Draw(renderer) || !renderer.EndFrame())
				{
					exitCode = 1;
					break;
				}
			}
		}
	}

	renderer.Shutdown();
	if (!application.Shutdown())
	{
		exitCode = 1;
	}

	return exitCode;
}
