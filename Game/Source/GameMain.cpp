#include "FrameTimer.h"
#include "Input/InputDispatcher.h"
#include "PlayerObject.h"
#include "WindowsApplication.h"
#include "World.h"

#include <memory>

int main()
{
	WindowsApplication windowsApplication;
	if (!windowsApplication.Initialize())
	{
		// @TODO 실패 로그
		return 1;
	}

	World world;
	world.AddGameObject(std::make_unique<PlayerObject>(world));

	FrameTimer frameTimer;
	int exitCode = 0;

	while (windowsApplication.ProcessMessages(exitCode))
	{
		const float deltaTime = frameTimer.Tick();

		// 입력 바인딩을 먼저 실행한 뒤 일반 갱신을 한다.
		world.GetInputDispatcher().Dispatch(windowsApplication.GetInputState(), deltaTime);
		world.Tick(deltaTime);

		// TODO: 렌더링을 연결한다.
	}

	return exitCode;
}
