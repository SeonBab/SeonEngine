#include "World.h"

#include "Component/CameraComponent.h"
#include "GameObject.h"

#include <utility>

World::World()
	: renderScene(*this)
{
}

World::~World() = default;

InputDispatcher& World::GetInputDispatcher()
{
	return inputDispatcher;
}

RenderScene& World::GetRenderScene()
{
	return renderScene;
}

GameObject* World::AddGameObject(std::unique_ptr<GameObject> newGameObject)
{
	if (newGameObject == nullptr) { return nullptr; }
	if (&newGameObject->GetWorld() != this) { return nullptr; }

	GameObject* gameObject = newGameObject.get();
	gameObjects.push_back(std::move(newGameObject));
	return gameObject;
}

bool World::SetActiveCamera(CameraComponent* camera)
{
	if (camera == nullptr)
	{
		activeCamera = nullptr;
		return true;
	}

	const GameObject* owner = camera->GetGameObject();
	if (owner == nullptr) { return false; }
	if (&owner->GetWorld() != this) { return false; }

	// 월드가 소유한 객체인지 확인한다. 등록 전 객체는 월드보다 먼저 사라질 수 있다.
	for (const std::unique_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject.get() == owner)
		{
			activeCamera = camera;
			return true;
		}
	}
	return false;
}

void World::Tick(float deltaTime)
{
	for (const std::unique_ptr<GameObject>& gameObject : gameObjects)
	{
		gameObject->Tick(deltaTime);
	}
}
