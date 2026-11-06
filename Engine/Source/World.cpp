#include "World.h"

#include "GameObject.h"

#include <utility>

World::World() = default;

World::~World() = default;

InputDispatcher& World::GetInputDispatcher()
{
	return inputDispatcher;
}

GameObject* World::AddGameObject(std::unique_ptr<GameObject> newGameObject)
{
	if (newGameObject == nullptr) { return nullptr; }
	if (&newGameObject->GetWorld() != this) { return nullptr; }

	GameObject* gameObject = newGameObject.get();
	gameObjects.push_back(std::move(newGameObject));
	return gameObject;
}

void World::Tick(float deltaTime)
{
	for (const std::unique_ptr<GameObject>& gameObject : gameObjects)
	{
		gameObject->Tick(deltaTime);
	}
}
