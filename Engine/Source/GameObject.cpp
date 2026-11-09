#include "GameObject.h"

#include "Component/Component.h"
#include "Component/TransformComponent.h"

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
