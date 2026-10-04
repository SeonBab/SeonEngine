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

	while (true)
	{
		window.PumpMessages();
		// 닫기 요청은 펌프 안에서 창 프로시저가 기록하므로 펌프 직후에 확인한다
		if (window.IsCloseRequested())
		{
			break;
		}
	}

	window.Shutdown();
	return 0;
}
