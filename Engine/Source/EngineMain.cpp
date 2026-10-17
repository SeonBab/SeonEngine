#include "GameEntry.h"
#include "Renderer/Renderer.h"
#include "Windows/WindowsApplication.h"
#include "World.h"

#include <chrono>

int main()
{
	const FWindowDescription description{};
	FWindowsApplication application;
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
	// 게임 구성 실패와 정상 종료 모두 World를 Renderer보다 먼저 정리한다.
	{
		UWorld world;
		if (!ConfigureGameWorld(world) || !world.CreateRenderStates(renderer))
		{
			exitCode = 1;
		}
		else
		{
			auto previousTime = std::chrono::steady_clock::now();
			while (application.PumpMessages(exitCode))
			{
				// 최소화 중에도 기준 시각을 갱신해 복원 시 정지 시간 전체를 전달하지 않는다.
				const auto currentTime = std::chrono::steady_clock::now();
				const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
				previousTime = currentTime;

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
				world.Tick(deltaSeconds);
				if (!world.SendAllEndOfFrameUpdates(renderer) || !world.GetScene().RenderFrame(renderer, clientWidth, clientHeight))
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
