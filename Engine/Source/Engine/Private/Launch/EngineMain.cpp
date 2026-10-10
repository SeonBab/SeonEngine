#include "Launch/EngineMain.h"

#include "Engine/Engine.h"
#include "Launch/LaunchEngineLoop.h"
#include "Logging/LogMacros.h"
#include "Platform/GenericPlatform/GenericWindow.h"
#include "Renderer/MeshRenderData.h"
#include "Renderer/RendererInterface.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogEngine);
}

int EngineMain(FGenericWindow& window, IRenderer& renderer)
{
	// TODO(seon): 실제 프레임 진행과 창 / 출력 책임을 목표 실행 구조에 맞춰 단계적으로 옮긴다.
	if (!window.Initialize())
	{
		return 1;
	}

	FEngineLoop engineLoop;
	engineLoop.Init();

	TArray<FMeshRenderData> renderData;

	// 객체가 존재해도 시작 최소화 상태에서는 그래픽스 초기화를 아직 하지 않았을 수 있다.
	bool bRendererInitialized = false;
	while (true)
	{
		window.PumpMessages();
		// 닫기 요청은 펌프 안에서 창 프로시저가 기록하므로 펌프 직후에 확인한다
		if (window.IsCloseRequested())
		{
			break;
		}

		engineLoop.Tick();

		// 유휴 대기 정책과 별개로 크기 0인 출력 자원을 만들거나 최소화 상태에 제출하지 않는다.
		const uint32 width = window.GetClientWidth();
		const uint32 height = window.GetClientHeight();
		if (window.IsMinimized() || width == 0 || height == 0) { continue; }

		if (!bRendererInitialized)
		{
			if (!renderer.Init(window.GetOSWindowHandle(), width, height))
			{
				// 실패한 구현이 부분 자원을 정리한다. 정상 종료용 Shutdown을 다시 호출하지 않는다.
				SE_LOG(LogEngine, Fatal, "Renderer initialization failed");
			}
			bRendererInitialized = true;
		}

		renderer.Resize(width, height);
		renderData.clear();
		const UEngine* engine = engineLoop.GetEngine();
		const FWorld* world = engine ? engine->GetWorld() : nullptr;
		if (world)
		{
			world->CollectMeshRenderData(renderData);
		}
		renderer.RenderFrame(renderData);
	}

	engineLoop.Exit();

	// 출력 자원을 정리하는 동안 OS 창을 유지한다. 출력 초기화 전에 닫혔다면 창만 정리한다.
	if (bRendererInitialized)
	{
		renderer.Shutdown();
	}
	window.Shutdown();
	return 0;
}
