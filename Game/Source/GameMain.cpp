#include "AssetManager.h"
#include "Component/CameraComponent.h"
#include "Component/DirectionalLightComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/TransformComponent.h"
#include "Core/Math/EulerAngles.h"
#include "Core/Math/Vector3.h"
#include "Engine.h"
#include "GameObject.h"
#include "PlayerObject.h"
#include "Renderer/Material.h"
#include "Texture/Texture.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
	// 저장소 루트 기준 경로다.
	constexpr std::string_view TeapotMeshPath = "Game/Content/Meshes/UtahTeapot/UtahTeapot.gltf";
	constexpr std::string_view CheckerTexturePath = "Engine/Content/Textures/Checker.png";

	/** 위에서 아래로 비스듬히 빛을 보내는 방향광을 World에 추가한다. */
	void AddDirectionalLight(World& world)
	{
		std::unique_ptr<GameObject> lightObject = std::make_unique<GameObject>(world);

		TransformComponent* transform = lightObject->GetComponent<TransformComponent>();
		transform->SetRotation(EulerAngles{ .pitch = 45.0f, .yaw = -30.0f, .roll = 0.0f });

		std::unique_ptr<DirectionalLightComponent> light = std::make_unique<DirectionalLightComponent>();
		light->SetColor(Vector3{ 1.0f, 1.0f, 1.0f });
		light->SetIntensity(1.0f);

		lightObject->AddComponent(std::move(light));
		world.AddGameObject(std::move(lightObject));
	}

	/**
	 * model의 배치표대로 객체들을 만들어, position(cm)에 놓은 빈 루트 객체 아래에 붙인다.
	 * 메시가 있는 노드에는 모두 material을 쓴다. 루트 객체를 반환한다.
	 */
	GameObject* AddModel(World& world, const Model& model, const Vector3& position, const std::shared_ptr<const Material>& material)
	{
		// 루트는 놓을 위치를 맡고, 노드 객체는 파일에 적힌 배치를 그대로 유지한다.
		std::unique_ptr<GameObject> rootObject = std::make_unique<GameObject>(world);
		rootObject->GetComponent<TransformComponent>()->SetPosition(position);
		GameObject* root = world.AddGameObject(std::move(rootObject));

		// 배치표 한 줄마다 객체를 만들어 등록한다. nodeObjects의 번호는 배치표 번호와 같다.
		std::vector<GameObject*> nodeObjects;
		nodeObjects.reserve(model.sceneNodes.size());
		for (const GLTFSceneNode& sceneNode : model.sceneNodes)
		{
			std::unique_ptr<GameObject> nodeObject = std::make_unique<GameObject>(world);
			nodeObject->GetComponent<TransformComponent>()->SetTransform(sceneNode.localTransform);

			if (sceneNode.mesh.has_value())
			{
				std::unique_ptr<StaticMeshComponent> mesh = std::make_unique<StaticMeshComponent>();
				mesh->SetStaticMesh(model.meshes[sceneNode.mesh.value()]);
				mesh->SetMaterial(material);
				nodeObject->AddComponent(std::move(mesh));
			}

			nodeObjects.push_back(world.AddGameObject(std::move(nodeObject)));
		}

		// SetParent는 등록된 객체끼리만 이을 수 있어 모두 등록한 뒤에 잇는다.
		// 최상위 노드는 루트에 붙인다. 순환은 로더가 이미 걸렀다.
		for (std::size_t i = 0; i < model.sceneNodes.size(); ++i)
		{
			const std::optional<std::size_t>& parentIndex = model.sceneNodes[i].parent;
			GameObject* parent = parentIndex.has_value() ? nodeObjects[parentIndex.value()] : root;
			nodeObjects[i]->SetParent(parent);
		}

		return root;
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

	AddDirectionalLight(world);

	Material materialData;
	materialData.baseColorTexture = AssetManager::Get().LoadTexture(CheckerTexturePath, TextureFormat::RGBA8SRGB);
	// 이번 주전자는 비금속 표면으로 지정한다.
	materialData.metallic = 0.0f;

	const std::shared_ptr<const Material> teapotMaterial = std::make_shared<const Material>(std::move(materialData));

	// 플레이어 앞에 좌우로 놓는다. 두 주전자는 한 번 불러온 모델의 메시를 공유한다.
	const std::shared_ptr<const Model> teapotModel = AssetManager::Get().LoadModel(TeapotMeshPath);
	if (teapotModel != nullptr)
	{
		AddModel(world, *teapotModel, Vector3{ -1000.0f, 0.0f, 3000.0f }, teapotMaterial);
		AddModel(world, *teapotModel, Vector3{ 1000.0f, 0.0f, 3000.0f }, teapotMaterial);
	}

	return engine.Run();
}
