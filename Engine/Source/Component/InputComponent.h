#pragma once

#include "Component.h"
#include "Input/Key.h"
#include "Input/KeyBinding.h"

#include <vector>

class InputDispatcher;

/**
 * 키 바인딩을 보관하고, 전달자가 요청한 키와 조건에 맞는 함수를 실행한다.
 * 부착 시 소속 World의 InputDispatcher에 추가되고 소멸 시 제거된다. InputState를 직접 읽지 않는다.
 */
class InputComponent : public Component
{
	// 생성과 소멸
public:
	InputComponent() = default;

	/** 추가된 전달자가 있으면 그곳에서 이 컴포넌트를 제거한다. */
	~InputComponent() override;

	// 등록
public:
	/** 소속 World의 전달자에 이 컴포넌트를 추가한다. GameObject::AddComponent가 호출한다. */
	void OnRegister() override;

private:
	// 소멸 시 GameObject를 거치지 않고 제거하려고 보관한다. 비소유이며 World가 이 컴포넌트보다 오래 산다.
	InputDispatcher* registeredDispatcher = nullptr;

	// 바인딩
public:
	/**
	 * 키와 조건에 실행할 함수를 추가한다. 같은 키와 조건에 여러 개를 추가하면 추가 순서로 실행한다.
	 * Unknown 키나 범위 밖 키, 빈 함수는 추가하지 않고 false를 반환한다. 입력 전달 중에는 호출하지 않는다.
	 */
	bool BindKey(Key key, KeyTrigger trigger, KeyAction action);

	/** key와 trigger가 맞는 바인딩을 추가 순서대로 실행한다. InputDispatcher가 호출한다. 콜백 예외는 전파한다. */
	void ExecuteBindings(Key key, KeyTrigger trigger, float deltaTime) const;

private:
	std::vector<KeyBinding> bindings;
};
