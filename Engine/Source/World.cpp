#include "World.h"

#include "Component/CameraComponent.h"
#include "GameObject.h"

#include <algorithm>
#include <utility>

World::World() = default;
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

bool World::DestroyGameObject(GameObject* gameObject)
{
	if (!ContainsGameObject(gameObject)) { return false; }

	std::vector<GameObject*> targets{ gameObject };
	for (std::size_t index = 0; index < targets.size(); ++index)
	{
		for (GameObject* child : targets[index]->GetChildren())
		{
			targets.push_back(child);
		}
	}

	std::vector<std::unique_ptr<GameObject>> removedObjects;
	removedObjects.reserve(targets.size());

	const auto isTarget = [&targets](const GameObject* object)
	{
		return std::find(targets.begin(), targets.end(), object) != targets.end();
	};

	// 소멸 중 끊긴 부모나 자식 포인터가 남지 않도록 모든 대상의 연결을 끊는다.
	for (GameObject* target : targets)
	{
		target->SetParent(nullptr);
	}

	// World의 목록을 정리한 뒤 함수 끝에서 객체를 소멸시킨다.
	for (std::unique_ptr<GameObject>& entry : gameObjects)
	{
		if (isTarget(entry.get()))
		{
			removedObjects.push_back(std::move(entry));
		}
	}
	std::erase(gameObjects, nullptr);

	return true;
}

bool World::ContainsGameObject(const GameObject* gameObject) const
{
	for (const std::unique_ptr<GameObject>& entry : gameObjects)
	{
		if (entry.get() == gameObject) { return true; }
	}

	return false;
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
