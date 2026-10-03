#include "Launch/EngineMain.h"

#include "Platform/Window.h"

int EngineMain()
{
	// TODO(seon): FEngine을 구현하면 창 생성, 메인 루프, 정리를 FEngine으로 옮긴다
	FWindow window;
	if (!window.Initialize())
	{
		return 1;
	}

	window.Shutdown();
	return 0;
}
