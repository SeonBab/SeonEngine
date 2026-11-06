#include "Component/CameraComponent.h"
#include "Engine.h"
#include "PlayerObject.h"

#include <memory>

int main()
{
	Engine engine;
	if (!engine.Initialize())
	{
		// @TODO 실패 로그
		return 1;
	}

	World& world = engine.GetWorld();
	GameObject* player = world.AddGameObject(std::make_unique<PlayerObject>(world));
	// World에 등록된 객체의 카메라만 지정할 수 있으므로 AddGameObject 뒤에 지정한다.
	if (player != nullptr) { world.SetActiveCamera(player->GetComponent<CameraComponent>()); }

	return engine.Run();
}
