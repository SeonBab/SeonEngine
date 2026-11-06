#include "InputComponent.h"

#include "GameObject.h"
#include "Input/InputDispatcher.h"
#include "World.h"

#include <utility>

InputComponent::~InputComponent()
{
	if (registeredDispatcher != nullptr)
	{
		registeredDispatcher->RemoveInputComponent(this);
	}
}

void InputComponent::OnRegister()
{
	GameObject* owner = GetGameObject();
	if (owner == nullptr) { return; }

	InputDispatcher& dispatcher = owner->GetWorld().GetInputDispatcher();
	dispatcher.AddInputComponent(this);
	registeredDispatcher = &dispatcher;
}

bool InputComponent::BindKey(Key key, KeyTrigger trigger, KeyAction action)
{
	if (key == Key::Unknown || key >= Key::Count) { return false; }
	if (!action) { return false; }

	bindings.push_back(KeyBinding{ key, trigger, std::move(action) });
	return true;
}

void InputComponent::ExecuteBindings(Key key, KeyTrigger trigger, float deltaTime) const
{
	for (const KeyBinding& binding : bindings)
	{
		if (binding.key != key || binding.trigger != trigger) { continue; }

		binding.action(deltaTime);
	}
}
