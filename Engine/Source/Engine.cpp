#include "Engine.h"

#include "Core/FrameTimer.h"
#include "Input/InputDispatcher.h"
#include "Renderer/SceneRenderData.h"

bool Engine::Initialize()
{
	if (!windowsApplication.Initialize()) { return false; }

	canRun = true;
	return true;
}

World& Engine::GetWorld()
{
	return world;
}

int Engine::Run()
{
	if (!canRun) { return 1; }
	canRun = false;

	// 초기화와 World 구성 시간을 첫 측정에서 빼려고 루프 직전에 만든다.
	FrameTimer frameTimer;
	int exitCode = 0;
	while (windowsApplication.ProcessMessages(exitCode))
	{
		Tick(frameTimer.Tick());
	}
	return exitCode;
}

void Engine::Tick(float deltaTime)
{
	// 입력 바인딩을 먼저 실행한 뒤 일반 갱신을 한다.
	world.GetInputDispatcher().Dispatch(windowsApplication.GetInputState(), deltaTime);
	world.Tick(deltaTime);

	// 모든 갱신이 끝난 뒤 이번 프레임 렌더 데이터를 모은다. 모으지 못하면(최소화 등) 이번 프레임은 그리지 않는다.
	int clientWidth = 0;
	int clientHeight = 0;
	if (!windowsApplication.GetClientSize(clientWidth, clientHeight)) { return; }

	SceneRenderData sceneRenderData;
	if (!world.GetRenderScene().CollectSceneRenderData(clientWidth, clientHeight, sceneRenderData)) { return; }

	// TODO: 렌더러를 연결해 같은 크기로 sceneRenderData를 그린다.
}
