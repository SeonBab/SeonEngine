#include "Input/InputDispatcher.h"

#include "Component/InputComponent.h"
#include "Input/InputState.h"
#include "Input/Key.h"
#include "Input/KeyBinding.h"

#include <algorithm>
#include <cstddef>

namespace
{
	KeyTrigger ToKeyTrigger(KeyEventType keyEventType)
	{
		if (keyEventType == KeyEventType::Pressed) { return KeyTrigger::Pressed; }

		return KeyTrigger::Released;
	}
}

void InputDispatcher::AddInputComponent(InputComponent* inputComponent)
{
	if (inputComponent == nullptr) { return; }

	const bool isAdded = std::find(inputComponents.begin(), inputComponents.end(), inputComponent) != inputComponents.end();
	if (isAdded) { return; }

	inputComponents.push_back(inputComponent);
}

void InputDispatcher::RemoveInputComponent(InputComponent* inputComponent)
{
	auto found = std::find(inputComponents.begin(), inputComponents.end(), inputComponent);
	if (found == inputComponents.end()) { return; }

	// 남은 컴포넌트의 추가 순서를 유지하려고 erase를 쓴다.
	inputComponents.erase(found);
}

void InputDispatcher::Dispatch(const InputState& inputState, float deltaTime) const
{
	// 이벤트를 바깥 반복에 두어, 키가 달라도 앞선 이벤트를 모든 컴포넌트가 처리한 뒤 다음 이벤트로 넘어간다.
	for (const KeyEvent& keyEvent : inputState.GetFrameEvents())
	{
		const KeyTrigger trigger = ToKeyTrigger(keyEvent.type);
		for (const InputComponent* inputComponent : inputComponents)
		{
			inputComponent->ExecuteBindings(keyEvent.key, trigger, deltaTime);
		}
	}

	// Held는 이벤트가 아니라 전달 시점의 눌림 상태로 정하므로, 한 프레임 안에 눌렀다 뗀 키는 실행하지 않는다.
	for (std::size_t index = 0; index < KeyCount; ++index)
	{
		const Key key = static_cast<Key>(index);
		if (!inputState.IsKeyDown(key)) { continue; }

		for (const InputComponent* inputComponent : inputComponents)
		{
			inputComponent->ExecuteBindings(key, KeyTrigger::Held, deltaTime);
		}
	}
}
