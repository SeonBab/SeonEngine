#include "AssetManager.h"
#include "Component/CameraComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/TransformComponent.h"
#include "Core/Math/Vector3.h"
#include "Engine.h"
#include "GameObject.h"
#include "PlayerObject.h"

#include <memory>
#include <string_view>
#include <utility>

namespace
{
	// 저장소 루트 기준 경로다.
	constexpr std::string_view TeapotMeshPath = "Game/Content/Meshes/UtahTeapot/UtahTeapot.gltf";

	/** position(cm)에 주전자 객체를 World에 추가한다. 메시를 읽지 못하면 그리지 않는 객체가 된다. */
	void AddTeapot(World& world, const Vector3& position)
	{
		std::unique_ptr<GameObject> teapot = std::make_unique<GameObject>(world);

		std::unique_ptr<TransformComponent> transform = std::make_unique<TransformComponent>();
		transform->SetPosition(position);
		teapot->AddComponent(std::move(transform));

		std::unique_ptr<StaticMeshComponent> mesh = std::make_unique<StaticMeshComponent>();
		mesh->SetStaticMesh(AssetManager::Get().LoadStaticMesh(TeapotMeshPath));
		teapot->AddComponent(std::move(mesh));

		world.AddGameObject(std::move(teapot));
	}
}

int main()
{
	Engine engine;
	if (!engine.Initialize())
	{
		return 1;
	}

	World& world = engine.GetWorld();
	GameObject* player = world.AddGameObject(std::make_unique<PlayerObject>(world));
	// World에 등록된 객체의 카메라만 지정할 수 있으므로 AddGameObject 뒤에 지정한다.
	if (player != nullptr) { world.SetActiveCamera(player->GetComponent<CameraComponent>()); }

	// 플레이어 앞 3m에 좌우로 놓는다. 같은 경로를 두 번 요청하므로 두 번째는 캐시의 메시를 공유한다.
	AddTeapot(world, Vector3{ -100.0f, 0.0f, 300.0f });
	AddTeapot(world, Vector3{ 100.0f, 0.0f, 300.0f });

	return engine.Run();
}
