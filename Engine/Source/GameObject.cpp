#include "GameObject.h"

#include "Component/Component.h"
#include "Component/TransformComponent.h"
#include "World.h"

#include <algorithm>
#include <utility>

GameObject::GameObject(World& targetWorld)
	: world(targetWorld)
{
	AddComponent(std::make_unique<TransformComponent>());
}

GameObject::~GameObject() = default;

World& GameObject::GetWorld() const
{
	return world;
}

bool GameObject::SetParent(GameObject* newParent)
{
	if (newParent == parent) { return true; }

	if (newParent != nullptr)
	{
		if (!world.ContainsGameObject(this) || !world.ContainsGameObject(newParent))
		{
			return false;
		}

		const GameObject* ancestor = newParent;
		while (ancestor != nullptr)
		{
			if (ancestor == this) { return false; }

			ancestor = ancestor->parent;
		}

		// 할당에 실패해도 기존 관계가 유지되도록 새 목록에 먼저 추가한다.
		newParent->children.push_back(this);
	}

	if (parent != nullptr)
	{
		std::erase(parent->children, this);
	}

	parent = newParent;
	return true;
}

Component* GameObject::AddComponent(std::unique_ptr<Component> newComponent)
{
	if (newComponent == nullptr) { return nullptr; }

	Component* component = newComponent.get();
	components.push_back(std::move(newComponent));
	component->SetGameObject(this);
	component->OnRegister();

	return component;
}

void GameObject::Tick(float deltaTime)
{
	OnTick(deltaTime);

	for (const std::unique_ptr<Component>& component : components)
	{
		component->Tick(deltaTime);
	}
}

void GameObject::OnTick(float)
{

}
