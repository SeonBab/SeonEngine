#include "Component.h"

Component::Component()
{
}

Component::~Component() = default;

GameObject* Component::GetGameObject() const
{
	return gameObject;
}

void Component::SetGameObject(GameObject* targetGameObject)
{
	gameObject = targetGameObject;
}

void Component::OnRegister()
{

}

void Component::Tick(float)
{

}
